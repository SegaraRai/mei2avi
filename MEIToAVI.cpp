#define NOMINMAX

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "MEIToAVI.hpp"
#include "ApproxFraction.hpp"
#include "AVI.hpp"
#include "AVIBuilder.hpp"
#include "Fraction.hpp"
#include "Source/CachedSource.hpp"
#include "Source/MemorySource.hpp"
#include "Source/PartialSource.hpp"

#include <Windows.h>

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace std::literals;


namespace {
#pragma pack(push, 1)
  struct BITMAPINFOHEADER {
    std::uint32_t biSize;
    std::uint32_t biWidth;
    std::uint32_t biHeight;
    std::uint16_t biPlanes;
    std::uint16_t biBitCount;
    std::uint32_t biCompression;
    std::uint32_t biSizeImage;
    std::uint32_t biXPelsPerMeter;
    std::uint32_t biYPelsPerMeter;
    std::uint32_t biClrUsed;
    std::uint32_t biClrImportant;
  };

  static_assert(sizeof(BITMAPINFOHEADER) == 4 * 10);


  struct WAVEFORMATEX {
    std::uint16_t wFormatTag;
    std::uint16_t nChannels;
    std::uint32_t nSamplesPerSec;
    std::uint32_t nAvgBytesPerSec;
    std::uint16_t nBlockAlign;
    std::uint16_t wBitsPerSample;
  };

  static_assert(sizeof(WAVEFORMATEX) == 4 * 4);
#pragma pack(pop)


  void CheckError(SSystem::SError error, const std::string& message) {
    if (error != SSystem::SError::errSuccess) {
      throw std::runtime_error(message);
    }
  }


  class FrameImageSource : public SourceBase {
    ERISA::SGLMovieFilePlayer* mPtrMovieFilePlayer;
    std::size_t mFrameIndex;
    std::size_t mSize;

  public:
    FrameImageSource(ERISA::SGLMovieFilePlayer& movieFilePlayer, std::uint_fast32_t frameIndex) :
      mPtrMovieFilePlayer(&movieFilePlayer),
      mFrameIndex(frameIndex),
      mSize(0)
    {
      const auto size = mPtrMovieFilePlayer->CurrentFrame()->GetImageSize();
      mSize = size.w * size.h * 4;
    }

    std::streamsize GetSize() const override {
      return mSize;
    }

    void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override {
      mPtrMovieFilePlayer->SeekToFrame(mFrameIndex);
      const auto ptrCurrentFrame = mPtrMovieFilePlayer->CurrentFrame();
      // hack
      const auto ptrSmartImage = static_cast<SakuraGL::SGLSmartImage*>(ptrCurrentFrame);
      const auto ptrImageBuffer = ptrSmartImage->GetImage();
      std::memcpy(data, ptrImageBuffer->ptrBuffer + offset, size);
    }
  };


  class MeiVideoStream : public AVIBuilder::AVIStream {
    ERISA::SGLMovieFilePlayer& mMovieFilePlayer;
    CacheStorage& mCacheStorage;
    std::uint_fast32_t mNumFrames;
    std::uint_fast32_t mFrameDataSize;
    AVI::AVIStreamHeader mStrh;
    BITMAPINFOHEADER mStrf;
    std::shared_ptr<MemorySource> mStrfMemorySource;

  public:
    MeiVideoStream(ERISA::SGLMovieFilePlayer& movieFilePlayer, CacheStorage& cacheStorage, const AVI::AVIStreamHeader& strh) :
      mMovieFilePlayer(movieFilePlayer),
      mCacheStorage(cacheStorage),
      mNumFrames(static_cast<std::uint_fast32_t>(mMovieFilePlayer.GetAllFrameCount())),
      mFrameDataSize(0),
      mStrh(strh),
      mStrf{},
      mStrfMemorySource()
    {
      const auto size = mMovieFilePlayer.CurrentFrame()->GetImageSize();
      mFrameDataSize = size.w * size.h * 4;

      mStrf = BITMAPINFOHEADER{
        sizeof(BITMAPINFOHEADER),
        static_cast<std::uint32_t>(size.w),
        static_cast<std::uint32_t>(-size.h),
        1u,
        32u,
        0u,   // BI_RGB
        mFrameDataSize,
        0u,
        0u,
        0u,
        0u,
      };
      mStrfMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&mStrf), sizeof(mStrf));
    }

    std::uint32_t GetFourCC() const override {
      // dbではなくdcの模様
      return FourCCdc;
    }

    std::uint_fast32_t CountStreams() const override {
      return static_cast<std::uint_fast32_t>(mMovieFilePlayer.GetAllFrameCount());
    }

    BlockInfo GetBlockInfo(std::uint_fast32_t index) const override {
      return BlockInfo{
        mFrameDataSize,
        index,
        1,
        AVI::AVIIF_KEYFRAME,
      };
    }

    std::shared_ptr<SourceBase> GetBlockData(std::uint_fast32_t index) const override {
      return std::make_shared<CachedSource>(mCacheStorage, std::make_shared<FrameImageSource>(mMovieFilePlayer, index));
    }

    AVI::AVIStreamHeader GetStrh() override {
      return mStrh;
    }

    std::shared_ptr<SourceBase> GetStrf() override {
      return mStrfMemorySource;
    }
  };


  class MeiAudioStream : public AVIBuilder::AVIStream {
    std::uint_fast32_t mAudioBlockSample;
    std::uint_fast32_t mBitsPerSample;
    std::uint_fast32_t mNumChannels;
    std::uint_fast32_t mSamplingRate;
    std::uint_fast32_t mBlockSize;
    std::uint_fast32_t mNumSamples;
    std::uint_fast32_t mNumBlocks;
    AVI::AVIStreamHeader mStrh;
    WAVEFORMATEX mStrf;
    std::shared_ptr<MemorySource> mStrfMemorySource;
    std::shared_ptr<MemorySource> mFullAudioMemorySource;
    std::vector<std::shared_ptr<SourceBase>> mBlockSources;

  public:
    MeiAudioStream(std::shared_ptr<std::uint8_t[]> audioData, std::size_t audioDataSize, std::uint_fast32_t audioBlockSample, std::uint_fast32_t bitsPerSample, std::uint_fast32_t numChannels, std::uint_fast32_t samplingRate) :
      mAudioBlockSample(audioBlockSample),
      mBitsPerSample(bitsPerSample),
      mNumChannels(numChannels),
      mSamplingRate(samplingRate),
      mBlockSize(mBitsPerSample / 8 * mNumChannels),
      mNumSamples(audioDataSize / mBlockSize),
      mNumBlocks((mNumSamples + audioBlockSample - 1) / audioBlockSample),
      mStrh(),
      mStrf{},
      mStrfMemorySource(),
      mFullAudioMemorySource(std::make_shared<MemorySource>(audioData, audioDataSize)),
      mBlockSources()
    {
      assert(mNumBlocks != 0);

      mStrh = AVI::AVIStreamHeader{
        FourCCauds,
        AVI::GetFourCC("\1\0\0\0"),
        0u,
        0u,
        0u,
        0u,
        1u,
        mSamplingRate,
        0u,
        mNumSamples,
        0,
        0xFFFFFFFFu,
        mBlockSize,
        {},
      };

      mStrf = WAVEFORMATEX{
        0x0001u,    // WAVE_FORMAT_PCM
        static_cast<std::uint16_t>(mNumChannels),
        static_cast<std::uint32_t>(mSamplingRate),
        static_cast<std::uint32_t>(mSamplingRate * mBlockSize),
        static_cast<std::uint16_t>(mBlockSize),
        static_cast<std::uint16_t>(mBitsPerSample),
      };
      mStrfMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&mStrf), sizeof(mStrf));

      mBlockSources.reserve(mNumBlocks);

      const std::size_t audioBlockSize = mAudioBlockSample * mBlockSize;

      std::size_t offset = 0;
      for (std::size_t i = 0; i < mNumBlocks - 1; i++) {
        mBlockSources.push_back(std::make_shared<PartialSource>(mFullAudioMemorySource, offset, audioBlockSize));
        offset += audioBlockSize;
      }
      mBlockSources.push_back(std::make_shared<PartialSource>(mFullAudioMemorySource, offset, audioDataSize - offset));
    }

    std::uint32_t GetFourCC() const override {
      return FourCCwb;
    }

    std::uint_fast32_t CountStreams() const override {
      return mNumBlocks;
    }

    BlockInfo GetBlockInfo(std::uint_fast32_t index) const override {
      const auto size = mBlockSources[index]->GetSize();
      return BlockInfo{
        static_cast<std::uint_fast32_t>(size),
        index * mAudioBlockSample,
        static_cast<std::uint_fast32_t>(size / mBlockSize),
        AVI::AVIIF_KEYFRAME,
      };
    }

    std::shared_ptr<SourceBase> GetBlockData(std::uint_fast32_t index) const override {
      return mBlockSources[index];
    }

    AVI::AVIStreamHeader GetStrh() override {
      return mStrh;
    }

    std::shared_ptr<SourceBase> GetStrf() override {
      return mStrfMemorySource;
    }
  };
}



MEIToAVI::MEIToAVI(const std::wstring& filePath, const Options& options) :
  mCacheStorage(options.cacheStorageSize, options.cacheStorageLimit),
  mFile(),
  mMovieFilePlayer(),
  mAvi()
{
  // open file
  {
    auto file = SSystem::SFileOpener::DefaultNewOpenFile(filePath.c_str(), SSystem::SFileOpener::OpenFlag::modeRead | SSystem::SFileOpener::OpenFlag::shareRead);
    if (!file) {
      throw std::runtime_error("cannot open file"s);
    }
    mFile.reset(file);
  }

  // open as video
  CheckError(mMovieFilePlayer.OpenMovieFile(mFile.get(), false), "cannot open file as video"s);

  // get media
  const auto& mediaFile = mMovieFilePlayer.GetMediaFile();

  
  // check if the file has audio data
  bool hasAudio = false;
  if (!(options.flags & NoAudio)) {
    hasAudio = mediaFile.m_flagsRead & ERISA::SGLMediaFile::readSoundInfo;
  }


  // load audio
  std::shared_ptr<std::uint8_t[]> audioData;
  std::size_t audioDataSize;
  std::uint_fast32_t audioBitsPerSample = 0;
  std::uint_fast32_t audioNumChannels = 0;
  std::uint_fast32_t audioSamplingRate = 0;

  if (hasAudio) {
    SSystem::SFile fileForSound;
    CheckError(fileForSound.Open(filePath.c_str(), SSystem::SFileOpener::OpenFlag::modeRead | SSystem::SFileOpener::OpenFlag::shareRead), "cannot open file for audio"s);

    ERISA::SGLSoundFilePlayer soundFilePlayer;
    CheckError(soundFilePlayer.OpenSoundFile(&fileForSound, false), "cannot open file as audio"s);

    audioBitsPerSample = soundFilePlayer.GetBitsPerSample();
    audioNumChannels = soundFilePlayer.GetChannelCount();
    audioSamplingRate = soundFilePlayer.GetFrequency();

    const std::uint_fast32_t audioNumSamples = soundFilePlayer.GetTotalSampleCount();
    const std::uint_fast32_t audioBlockSize = audioBitsPerSample / 8 * audioNumChannels;

    // 音声データを全て読み出す
    // 予め読んでいるのは事前にチャンクの配置を決定しておく必要があるため
    // meiファイルでの配置をそのままAVIにするのも考えたがそれはそれで面倒そうだった

    audioDataSize = audioNumSamples * audioBlockSize;
    audioData = std::shared_ptr<std::uint8_t[]>(std::make_unique<std::uint8_t[]>(audioDataSize));

    SSystem::SArray<std::uint8_t> audioBuffer;
    std::uint_fast32_t offset = 0;
    while (offset < audioDataSize) {
      soundFilePlayer.GetNextWaveBuffer(audioBuffer);
      std::memcpy(audioData.get() + offset, audioBuffer.GetArray(), audioBuffer.GetLength());
      offset += audioBuffer.GetLength();

#ifdef _DEBUG
      OutputDebugStringW((L"Read audio: "s + std::to_wstring(offset) + L" / "s + std::to_wstring(audioDataSize) + L"\n"s).c_str());
#endif
    }

    soundFilePlayer.Close();
    fileForSound.Close();

    if (!audioDataSize) {
      hasAudio = false;
    }
  }


  // load video
  const bool videoHasAlpha = !(options.flags & NoAlpha) && mediaFile.m_eriInfoHeader.fdwFormatType == 0x04000001 /*ERI_RGBA_IMAGE*/;
  const auto videoSize = mMovieFilePlayer.CurrentFrame()->GetImageSize();
  const auto videoNumFrames = static_cast<std::uint_fast32_t>(mMovieFilePlayer.GetAllFrameCount());
  const auto videoDurationMillis = static_cast<std::uint_fast32_t>(mMovieFilePlayer.GetTotalTime());

  Fraction<std::uint_fast32_t> videoFPS{
    videoNumFrames * static_cast<std::uint_fast32_t>(1000),
    videoDurationMillis,
  };
  if (!(options.flags & NoApproxFPS)) {
    const auto orgFPS = videoFPS;

    videoFPS = ApproxFraction(videoFPS);

    if (!(options.flags & NoMessage)) {
      std::wcerr << L"[info] fps is approximated as "sv
                 << videoFPS.numerator << L"/"sv << videoFPS.denominator << L" ("sv << (static_cast<double>(videoFPS.numerator) / videoFPS.denominator) << L") from "sv
                 << orgFPS.numerator << L"/"sv << orgFPS.denominator << L" ("sv << (static_cast<double>(orgFPS.numerator) / orgFPS.denominator) << L")"sv <<std::endl;
    }
  }


  // check frame size
  if (!(options.flags & NoMessage)) {
    if (videoSize.w != mediaFile.m_eriInfoHeader.nImageWidth || videoSize.h != mediaFile.m_eriInfoHeader.nImageHeight) {
      std::wcerr << L"[warn] the resolution of the output video ("sv << videoSize.w << L"x"sv << videoSize.h
                 << L") will be different from the original ("sv << mediaFile.m_eriInfoHeader.nImageWidth << L"x"sv << mediaFile.m_eriInfoHeader.nImageHeight << L")"sv << std::endl;
    }
  }


  // 1フレームあたりのサンプル数
  // 全てのオーディオブロックはこの単位にする
  // ずれた場合はブロック内のサンプル数は変えずにブロックの位置を調整して合わせる
  std::uint_fast32_t audioSamplesPerFrame = 0;

  if (hasAudio) {
    audioSamplesPerFrame = options.audioBlockSamples ? options.audioBlockSamples : audioSamplingRate * videoFPS.denominator / videoFPS.numerator;
  }


  AVIBuilder aviBuilder;

  aviBuilder.SetJunkSize(options.junkChunkSize);

  auto listInfo = std::make_shared<RIFFList>(AVI::GetFourCC("LIST"), AVI::GetFourCC("INFO"));

  const char isftStr[] = "mei2avi v0.2.0";
  auto isftMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(isftStr), sizeof(isftStr));
  auto isft = std::make_shared<RIFFChunk>(AVI::GetFourCC("ISFT"), isftMemorySource);
  listInfo->AppendChild(isft);

  aviBuilder.SetListInfo(listInfo);

  aviBuilder.SetAvihFlags(AVI::AVIF_HASINDEX | AVI::AVIF_ISINTERLEAVED | AVI::AVIF_TRUSTCKTYPE);

  // video stream
  auto videoStream = std::make_shared<MeiVideoStream>(mMovieFilePlayer, mCacheStorage, AVI::AVIStreamHeader{
    AVI::GetFourCC("vids"),
    videoHasAlpha ? AVI::GetFourCC("RGBA") : AVI::GetFourCC("\0\0\0\0"),
    0u,
    0u,
    0u,
    0u,
    videoFPS.denominator,
    videoFPS.numerator,
    0u,
    videoNumFrames,
    0u,
    0xFFFFFFFFu,
    0u,
    {
      0u,
      0u,
      static_cast<std::uint16_t>(videoSize.w),
      static_cast<std::uint16_t>(videoSize.h),
    },
  });
  aviBuilder.AddStream(videoStream, true);

  // audio stream
  if (hasAudio) {
    auto audioStream = std::make_shared<MeiAudioStream>(audioData, audioDataSize, audioSamplesPerFrame, audioBitsPerSample, audioNumChannels, audioSamplingRate);
    aviBuilder.AddStream(audioStream, false);
  }


  mAvi = aviBuilder.BuildAVI();
}


SourceBase& MEIToAVI::GetSource() {
  return *mAvi;
}
