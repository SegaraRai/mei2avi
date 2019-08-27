#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <stdexcept>
#include <vector>

#include "AVIBuilder.hpp"
#include "Fraction.hpp"
#include "RIFF/RIFFChunk.hpp"
#include "RIFF/RIFFList.hpp"
#include "RIFF/RIFFRoot.hpp"
#include "Source/MemorySource.hpp"


// TODO:
// - RIFFList, RIFFRootをもっと高機能に
// - RIFFRootをRIFFListから派生させる
// - RIFFChunkに2の倍数パディングを実装

/*
RIFF-AVI
  LIST-hdrl
    avih
    LIST-strl
      strh
      strf : provide
        indx
      strn : provide
    LIST-strl
      strh
      strf : provide
      indx
      strn : provide
    LIST-odml
      dmlh
  JUNK
  LIST-INFO
  LIST-movi
    ****
    ****
    ...
    ix**
    ix**
  idx1
RIFF-AVIX
  LIST-movi
    ****
    ****
    ...
    ix**
    ix**
...
//*/


namespace {
  constexpr std::uint_fast32_t MaxRiffSizeAVI  = 0x40000000;    // 1 GiB
  constexpr std::uint_fast32_t MaxRiffSizeAVIX = 0x40000000;    // 1 GiB

  constexpr std::uint_fast32_t MaxBlocksAVI  = 0xFFFFFFFF;
  constexpr std::uint_fast32_t MaxBlocksAVIX = 0xFFFFFFFF;


  // 1 -> FourCC("01\0\0")
  std::uint32_t IndexToFourCC(unsigned int index) {
    if (index >= 100) {
      throw std::runtime_error("index too large");
    }
    std::uint32_t fourCC = 0;
    fourCC |= ('0' + index / 10) << 0;
    fourCC |= ('0' + index % 10) << 8;
    return fourCC;
  }
}


//


bool AVIBuilder::AVIStream::FixSuggestedBufferSize() const {
  return true;
}


std::shared_ptr<SourceBase> AVIBuilder::AVIStream::GetStrn() {
  return nullptr;
}


void AVIBuilder::AVIStream::OnFinishListStrl(std::shared_ptr<RIFFList> listStrl) {
  // do nothing
}


//


AVIBuilder::AVIBuilder(BuilderFlags builderFlags) :
  mBuilderFlags(builderFlags),
  mStreams(),
  mPrimaryVideoStreamIndex(),
  mAvihFlags(DefaultAvihFlags),
  mJunkSize(DefaultJunkSize),
  mListInfo()
{}


void AVIBuilder::SetAvihFlags(std::uint32_t avihFlags) {
  mAvihFlags = avihFlags;
}


void AVIBuilder::SetJunkSize(std::uint_fast32_t junkSize) {
  mJunkSize = junkSize;
}


void AVIBuilder::SetListInfo(std::shared_ptr<RIFFList> listInfo) {
  mListInfo = listInfo;
}


void AVIBuilder::AddStream(std::shared_ptr<AVIStream> stream, bool primaryVideoStream) {
  if (primaryVideoStream) {
    assert(!mPrimaryVideoStreamIndex);
    mPrimaryVideoStreamIndex.emplace(mStreams.size());
  }

  mStreams.push_back(stream);
}


std::uint_fast32_t AVIBuilder::CountTotalFrames() const {
  if (!mPrimaryVideoStreamIndex) {
    throw std::runtime_error("no primary video stream");
  }
  return mStreams[mPrimaryVideoStreamIndex.value()]->CountStreams();
}


std::uint32_t AVIBuilder::GetAvihMicroSecPerFrame() const {
  if (!mPrimaryVideoStreamIndex) {
    throw std::runtime_error("no primary video stream");
  }
  const auto strh = mStreams[mPrimaryVideoStreamIndex.value()]->GetStrh();
  return static_cast<std::uint32_t>(1.e6 * strh.dwScale / strh.dwRate + .5);
}


std::uint32_t AVIBuilder::GetAvihWidth() const {
  if (!mPrimaryVideoStreamIndex) {
    throw std::runtime_error("no primary video stream");
  }
  const auto strh = mStreams[mPrimaryVideoStreamIndex.value()]->GetStrh();
  return strh.rcFrame.right - strh.rcFrame.left;
}

std::uint32_t AVIBuilder::GetAvihHeight() const {
  if (!mPrimaryVideoStreamIndex) {
    throw std::runtime_error("no primary video stream");
  }
  const auto strh = mStreams[mPrimaryVideoStreamIndex.value()]->GetStrh();
  return strh.rcFrame.bottom - strh.rcFrame.top;
}


void AVIBuilder::OnFinishListHdrl(std::shared_ptr<RIFFList> listStrl) {
  // do nothing
}


void AVIBuilder::OnFinishListMovi(std::shared_ptr<RIFFList> listMovi, bool isAvix) {
  // do nothing
}


void AVIBuilder::OnFinishRiffAvi(std::shared_ptr<RIFFList> riffAvi, bool isAvix) {
  // do nothing
}


void AVIBuilder::OnFinishAll(RIFFRoot& riffRoot) {
  // do nothing
}


std::shared_ptr<SourceBase> AVIBuilder::BuildAVI() {
  struct StreamInfo {
    struct PerBlockInfo {
      std::size_t blockIndex;
      AVIStream::BlockInfo info;
      std::shared_ptr<SourceBase> data;
      std::shared_ptr<RIFFChunk> chunk;
    };

    struct PerRIFFInfo {
      bool isAvix;
      std::shared_ptr<RIFFList> riffAvi;
      std::shared_ptr<RIFFList> listMovi;
      std::shared_ptr<RIFFChunk> ixxx;      // ix00, ix01, ...
      std::shared_ptr<MemorySource> ixxxMemorySource;
      std::shared_ptr<RIFFBase> ixxxBaseRiff;
      std::uint_fast32_t duration;
      std::vector<PerBlockInfo> blocks;
    };

    std::shared_ptr<AVIStream> stream;
    std::size_t numBlocks;
    std::uint32_t fourCC;
    Fraction<std::uint_fast64_t> timeCoef;    // seconds / frame
    std::size_t currentBlockIndex;
    //
    std::shared_ptr<RIFFList> listStrl;
    std::shared_ptr<MemorySource> strhMemorySource;
    std::shared_ptr<RIFFChunk> strh;
    std::shared_ptr<RIFFChunk> strf;
    std::shared_ptr<RIFFChunk> strn;
    std::shared_ptr<MemorySource> indxMemorySource;
    std::shared_ptr<RIFFChunk> indx;
    //
    std::uint_fast32_t maxDataSize;
    std::uint_fast32_t maxBlocksPerSec;
    std::deque<std::uint_fast32_t> maxBytesPerSecBlockSizes;
    std::uint_fast32_t maxBytesPerSecSizeCount;
    std::uint_fast32_t maxBytesPerSec;
    //
    std::vector<PerRIFFInfo> riffs;
  };


  RIFFRoot riffRoot;

  std::vector<StreamInfo> streamInfoArray;

  if (mStreams.size() >= 100) {
    throw std::runtime_error("too many streams");
  }

  for (std::size_t i = 0; i < mStreams.size(); i++) {
    auto& stream = *mStreams[i];

    const auto strh = stream.GetStrh();

    if (strh.dwScale == 0) {
      throw std::runtime_error("dwScale must not be zero");
    }

    if (strh.dwRate == 0) {
      throw std::runtime_error("dwRate must not be zero");
    }

    const std::uint32_t fourCC = (stream.GetFourCC() & 0xFFFF0000) | IndexToFourCC(i);

    std::uint_fast32_t maxBlocksPerSec = (strh.dwRate + strh.dwScale - 1) / strh.dwScale;
    if (maxBlocksPerSec == 0) {
      maxBlocksPerSec = 1;
    }

    streamInfoArray.push_back(StreamInfo{
      mStreams[i],
      stream.CountStreams(),
      fourCC,
      Fraction<std::uint_fast64_t>(strh.dwScale, strh.dwRate),
      0,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      nullptr,
      0,
      maxBlocksPerSec,
      {},
      0,
      0,
      {},
    });
  }

  // ## RIFF-AVI
  auto riffAvi = std::make_shared<RIFFList>(&riffRoot, AVI::GetFourCC("RIFF"), AVI::GetFourCC("AVI "));
  riffRoot.AddChild(riffAvi);

  // ### LIST-hdrl
  auto listHdrl = std::make_shared<RIFFList>(riffAvi.get(), AVI::GetFourCC("LIST"), AVI::GetFourCC("hdrl"));
  riffAvi->AddChild(listHdrl);

  // #### avih
  AVI::MainAVIHeader avihData{
    GetAvihMicroSecPerFrame(),
    0u,   // TODO
    0u,
    mAvihFlags,
    0u,   // filled later
    0u,
    static_cast<std::uint32_t>(mStreams.size()),
    0u,   // filled later
    GetAvihWidth(),
    GetAvihHeight(),
  };
  auto avihMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&avihData), sizeof(avihData));
  auto avih = std::make_shared<RIFFChunk>(listHdrl.get(), AVI::GetFourCC("avih"), avihMemorySource);
  listHdrl->AddChild(avih);

  // #### LIST-strl
  for (std::size_t i = 0; i < mStreams.size(); i++) {
    auto& stream = *mStreams[i];

    // #### LIST-strl
    auto listStrl = std::make_shared<RIFFList>(listHdrl.get(), AVI::GetFourCC("LIST"), AVI::GetFourCC("strl"));
    listHdrl->AddChild(listStrl);

    streamInfoArray[i].listStrl = listStrl;

    // ##### strh
    auto strhData = stream.GetStrh();
    auto strhMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&strhData), sizeof(strhData));
    auto strh = std::make_shared<RIFFChunk>(listStrl.get(), AVI::GetFourCC("strh"), strhMemorySource);
    listStrl->AddChild(strh);

    streamInfoArray[i].strhMemorySource = strhMemorySource;
    streamInfoArray[i].strh = strh;

    // ##### strf
    auto strfSource = stream.GetStrf();
    auto strf = std::make_shared<RIFFChunk>(listStrl.get(), AVI::GetFourCC("strf"), strfSource);
    listStrl->AddChild(strf);

    streamInfoArray[i].strf = strf;

    // ##### strn
    auto strnSource = stream.GetStrn();
    if (strnSource) {
      auto strn = std::make_shared<RIFFChunk>(listStrl.get(), AVI::GetFourCC("strn"), strnSource);
      listStrl->AddChild(strn);

      streamInfoArray[i].strn = strn;
    }

    // ##### indx (set later)
    auto indx = std::make_shared<RIFFChunk>(listStrl.get(), AVI::GetFourCC("indx"));
    listStrl->AddChild(indx);

    streamInfoArray[i].indx = indx;

    stream.OnFinishListStrl(listStrl);
  }

  // Open-DML
  if (!(mBuilderFlags & NoOdml)) {
    // #### LIST-odml
    auto listOdml = std::make_shared<RIFFList>(listHdrl.get(), AVI::GetFourCC("LIST"), AVI::GetFourCC("odml"));
    listHdrl->AddChild(listOdml);

    // ##### dmlh
    AVI::AVIEXTHEADER dmlhData{
      static_cast<std::uint32_t>(CountTotalFrames()),
      {},
    };
    auto dmlhMemorySource = std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&dmlhData), sizeof(dmlhData));
    auto dmlh = std::make_shared<RIFFChunk>(listOdml.get(), AVI::GetFourCC("dmlh"), dmlhMemorySource);
    listOdml->AddChild(dmlh);
  }

  OnFinishListHdrl(listHdrl);

  //

  // ### LIST-INFO
  if (mListInfo) {
    riffAvi->AddChild(mListInfo);
  }

  // ### JUNK
  if (mJunkSize) {
    auto junk = std::make_shared<RIFFChunk>(riffAvi.get(), AVI::GetFourCC("JUNK"), std::make_shared<MemorySource>(mJunkSize));
    riffAvi->AddChild(junk);
  }

  // ### LIST-movi
  auto listMovi = std::make_shared<RIFFList>(riffAvi.get(), AVI::GetFourCC("LIST"), AVI::GetFourCC("movi"));
  riffAvi->AddChild(listMovi);

  // #### idx1
  auto idx1 = std::make_shared<RIFFChunk>(riffAvi.get(), AVI::GetFourCC("idx1"));
  if (!(mBuilderFlags & NoIdx1)) {
    riffAvi->AddChild(idx1);
  }

  // end of header

  std::shared_ptr<RIFFList> riffAvix = riffAvi;
  std::shared_ptr<RIFFList> avixListMovi = listMovi;

  std::vector<StreamInfo::PerRIFFInfo*> perRIFFInfoArray;
  perRIFFInfoArray.resize(mStreams.size());

  // idx1構築用
  struct BlockInfo {
    std::size_t streamIndex;
    std::size_t blockIndex;
    std::shared_ptr<RIFFChunk> chunk;
  };
  std::vector<BlockInfo> blocks;    // idx1構築用

  std::uint_fast32_t sizeCount = 0;

  std::uint_fast32_t maxChunkSize = 0;

  bool initializeRiff = true;

  while (true) {
    std::vector<std::size_t> remainingStreams;
    for (std::size_t i = 0; i < mStreams.size(); i++) {
      const auto& streamInfo = streamInfoArray[i];
      if (streamInfo.currentBlockIndex >= streamInfo.numBlocks) {
        continue;
      }
      remainingStreams.push_back(i);
    }

    const bool isAvix = riffAvix != riffAvi;
    const bool finished = remainingStreams.empty();
    bool startNextAvix = false;

    const auto maxBlocks = isAvix ? MaxBlocksAVIX : MaxBlocksAVI;
    const auto maxRiffSize = isAvix ? MaxRiffSizeAVIX : MaxRiffSizeAVI;

    // 次にブロックを置くストリーム
    const std::size_t nextStreamIndex = finished ? 0 : *std::min_element(remainingStreams.cbegin(), remainingStreams.cend(), [this, &streamInfoArray] (std::size_t a, std::size_t b) {
      const auto timeA = streamInfoArray[a].timeCoef * mStreams[a]->GetBlockInfo(streamInfoArray[a].currentBlockIndex).startTime;
      const auto timeB = streamInfoArray[b].timeCoef * mStreams[b]->GetBlockInfo(streamInfoArray[b].currentBlockIndex).startTime;
      return timeA == timeB ? a < b : timeA < timeB;
    });

    // 次のチャンクの大きさ
    const std::uint_fast32_t nextChunkSize = finished ? 0 : 8 + mStreams[nextStreamIndex]->GetBlockInfo(streamInfoArray[nextStreamIndex].currentBlockIndex).size;

    // finish this RIFF-AVI or RIFF-AVIX list
    if ((blocks.size() >= maxBlocks || sizeCount + nextChunkSize >= maxRiffSize || finished) && !initializeRiff) {
      // AVI-RIFFリストより後かつLIST-moviリストより前の領域の大きさがまだ増加する可能性があるので、
      // AVI-RIFFリストを基準にする場合は最後にインデックス全体のオフセットを書き直す必要が生じる
      // ここではLIST-moviリストを基準にすることでこの問題を避けている
      // 当然だがLIST-moviリスト内の構造に変化がある場合は上述の問題が生じる（ここでは大丈夫）

      auto baseRiff = avixListMovi;

      // ixxx (ix00, ix01, ...)
      for (std::size_t i = 0; i < mStreams.size(); i++) {
        const std::size_t ixxxSize = sizeof(AVI::AVISTDINDEX) + sizeof(AVI::AVISTDINDEXENTRY) * perRIFFInfoArray[i]->blocks.size();
        auto ixxxData = std::make_unique<std::uint8_t[]>(ixxxSize);
        *reinterpret_cast<AVI::AVISTDINDEX*>(ixxxData.get()) = AVI::AVISTDINDEX{
          2u,
          0u,
          1u,   // AVI_INDEX_OF_CHUNKS
          static_cast<std::uint32_t>(perRIFFInfoArray[i]->blocks.size()),
          streamInfoArray[i].fourCC,
          0u,   // filled later
          0u,
        };
        const auto ixxxEntries = reinterpret_cast<AVI::AVISTDINDEXENTRY*>(ixxxData.get() + sizeof(AVI::AVISTDINDEX));
        const auto baseOffset = baseRiff->GetOffset();
        for (std::size_t j = 0; j < perRIFFInfoArray[i]->blocks.size(); j++) {
          auto& chunk = *perRIFFInfoArray[i]->blocks[j].chunk;
          ixxxEntries[j] = AVI::AVISTDINDEXENTRY{
            static_cast<std::uint32_t>(chunk.GetOffset() - baseOffset + 8),
            static_cast<std::uint32_t>(chunk.GetSize() - 8),
          };
        }
        auto ixxxMemorySource = std::make_shared<MemorySource>(std::move(ixxxData), ixxxSize);
        auto ixxx = std::make_shared<RIFFChunk>(avixListMovi.get(), AVI::GetFourCC("ix\0\0") | ((streamInfoArray[i].fourCC & 0x0000FFFF) << 16), ixxxMemorySource);
        avixListMovi->AddChild(ixxx);
        perRIFFInfoArray[i]->ixxx = ixxx;
        perRIFFInfoArray[i]->ixxxMemorySource = ixxxMemorySource;
        perRIFFInfoArray[i]->ixxxBaseRiff = baseRiff;
      }

      if (!isAvix) {
        // idx1
        auto idx1MemorySource = std::make_shared<MemorySource>(sizeof(AVI::AVIINDEXENTRY) * blocks.size());
        const auto indexEntries = reinterpret_cast<AVI::AVIINDEXENTRY*>(idx1MemorySource->GetData());
        const auto baseOffset = avixListMovi->GetOffset() + 8;    // I don't know why +8, but FFmpeg does
        for (std::size_t i = 0; i < blocks.size(); i++) {
          const auto& block = blocks[i];
          indexEntries[i] = AVI::AVIINDEXENTRY{
            streamInfoArray[block.streamIndex].fourCC,
            mStreams[block.streamIndex]->GetBlockInfo(block.blockIndex).indexFlags,
            static_cast<std::uint32_t>(block.chunk->GetOffset() - baseOffset),            // relative to movi (absolute position is permitted also)
            static_cast<std::uint32_t>(block.chunk->GetSize() - 8),                       // I don't know why -8, but FFmpeg does
          };
        }
        idx1->SetContentSource(idx1MemorySource);

        // avih
        reinterpret_cast<AVI::MainAVIHeader*>(avihMemorySource->GetData())->dwTotalFrames = streamInfoArray[mPrimaryVideoStreamIndex.value()].currentBlockIndex + 1;    // I don't know why +1, but FFmpeg does
      }

      OnFinishRiffAvi(listMovi, isAvix);
      OnFinishRiffAvi(riffAvix, isAvix);

      startNextAvix = true;
    }

    // finish
    if (finished) {
      break;
    }

    // start new RIFF-AVIX list
    if (startNextAvix) {
      riffAvix = std::make_shared<RIFFList>(&riffRoot, AVI::GetFourCC("RIFF"), AVI::GetFourCC("AVIX"));
      riffRoot.AddChild(riffAvix);

      avixListMovi = std::make_shared<RIFFList>(riffAvix.get(), AVI::GetFourCC("LIST"), AVI::GetFourCC("movi"));
      riffAvix->AddChild(avixListMovi);

      initializeRiff = true;
    }

    // start of RIFF-AVI or RIFF-AVIX
    if (initializeRiff) {
      blocks.clear();

      sizeCount = static_cast<std::uint_fast32_t>(riffAvix->GetSize());

      for (std::size_t i = 0; i < mStreams.size(); i++) {
        streamInfoArray[i].riffs.push_back(StreamInfo::PerRIFFInfo{
          true,
          riffAvix,
          avixListMovi,
          nullptr,
          0,
          {},
        });

        perRIFFInfoArray[i] = &*(streamInfoArray[i].riffs.end() - 1);
      }

      initializeRiff = false;
    }

    // append data chunk

    auto stream = mStreams[nextStreamIndex];
    auto& streamInfo = streamInfoArray[nextStreamIndex];
    auto& perRIFFInfo = *perRIFFInfoArray[nextStreamIndex];

    auto chunkSource = stream->GetBlockData(streamInfo.currentBlockIndex);
    auto chunk = std::make_shared<RIFFChunk>(avixListMovi.get(), streamInfo.fourCC, chunkSource);
    avixListMovi->AddChild(chunk);

    blocks.push_back(BlockInfo{
      nextStreamIndex,
      streamInfo.currentBlockIndex,
      chunk,
    });

    const auto blockInfo = stream->GetBlockInfo(streamInfo.currentBlockIndex);
    perRIFFInfo.blocks.push_back(StreamInfo::PerBlockInfo{
      streamInfo.currentBlockIndex,
      blockInfo,
      chunkSource,
      chunk,
    });
    perRIFFInfo.duration += blockInfo.duration;
    streamInfo.currentBlockIndex++;

    // 最大サイズチェック
    const auto chunkSize = chunk->GetSize();

    // AVI全体の最大チャンクサイズ
    // とりあえずデータチャンクだけ調べる
    // データチャンクより大きいチャンクが存在すると間違った結果になるが実用上問題ないと判断
    maxChunkSize = std::max(maxChunkSize, static_cast<std::uint_fast32_t>(chunkSize));

    // ストリームの最大データサイズ
    streamInfo.maxDataSize = std::max(streamInfo.maxDataSize, static_cast<std::uint_fast32_t>(chunkSize - 8));

    // ストリームの最大転送レート計算
    streamInfo.maxBytesPerSecBlockSizes.push_back(static_cast<std::uint_fast32_t>(chunkSize));
    streamInfo.maxBytesPerSecSizeCount += static_cast<std::uint_fast32_t>(chunkSize);
    if (streamInfo.maxBytesPerSecBlockSizes.size() > streamInfo.maxBlocksPerSec) {
      streamInfo.maxBytesPerSecSizeCount -= streamInfo.maxBytesPerSecBlockSizes[0];
      streamInfo.maxBytesPerSecBlockSizes.pop_front();
    }
    streamInfo.maxBytesPerSec = std::max(streamInfo.maxBytesPerSec, streamInfo.maxBytesPerSecSizeCount);

    //

    sizeCount += static_cast<std::uint_fast32_t>(chunk->GetSize());
  }

  // set indx chunks
  std::vector<std::pair<std::uint64_t*, RIFFChunk*>> superIndexEntries;   // 後でqwOffsetを修正する用
  for (std::size_t i = 0; i < mStreams.size(); i++) {
    auto& streamInfo = streamInfoArray[i];

    const std::size_t indxSize = sizeof(AVI::AVISUPERINDEX) + sizeof(AVI::AVISUPERINDEXENTRY) * streamInfo.riffs.size();
    auto indxData = std::make_unique<std::uint8_t[]>(indxSize);
    *reinterpret_cast<AVI::AVISUPERINDEX*>(indxData.get()) = AVI::AVISUPERINDEX{
      4u,
      0u,
      0u,   // AVI_INDEX_OF_CHUNKS
      streamInfo.riffs.size(),
      streamInfo.fourCC,
      {},
    };

    const auto indxEntries = reinterpret_cast<AVI::AVISUPERINDEXENTRY*>(indxData.get() + sizeof(AVI::AVISUPERINDEX));
    for (std::size_t j = 0; j < streamInfo.riffs.size(); j++) {
      auto& perRiffInfo = streamInfo.riffs[j];
      indxEntries[j] = AVI::AVISUPERINDEXENTRY{
        0u,   // filled later
        static_cast<std::uint32_t>(perRiffInfo.ixxx->GetSize()),   // includes the size of a chunk header
        static_cast<std::uint32_t>(perRiffInfo.duration),
      };
    }

    auto indxMemorySource = std::make_shared<MemorySource>(std::move(indxData), indxSize);
    streamInfo.indx->SetContentSource(indxMemorySource);
    streamInfo.indxMemorySource = indxMemorySource;
  }

  // 骨組み完成

  // fix offsets
  for (std::size_t i = 0; i < mStreams.size(); i++) {
    auto& streamInfo = streamInfoArray[i];

    // fix qwBaseOffset (ixxx, standard index)
    for (std::size_t j = 0; j < streamInfo.riffs.size(); j++) {
      auto& perRiffInfo = streamInfo.riffs[j];
      reinterpret_cast<AVI::AVISTDINDEX*>(perRiffInfo.ixxxMemorySource->GetData())->qwBaseOffset = static_cast<std::uint64_t>(perRiffInfo.ixxxBaseRiff->GetOffset());
    }

    // fix qwOffset (indx, super index)
    auto indxMemorySource = streamInfo.indxMemorySource;
    const auto indxEntries = reinterpret_cast<AVI::AVISUPERINDEXENTRY*>(indxMemorySource->GetData() + sizeof(AVI::AVISUPERINDEX));
    for (std::size_t j = 0; j < streamInfo.riffs.size(); j++) {
      auto& perRiffInfo = streamInfo.riffs[j];
      indxEntries[j].qwOffset = static_cast<std::uint64_t>(perRiffInfo.ixxx->GetOffset());
    }
  }

  // fix strh
  for (std::size_t i = 0; i < mStreams.size(); i++) {
    auto& stream = mStreams[i];
    auto& streamInfo = streamInfoArray[i];

    if (!stream->FixSuggestedBufferSize()) {
      continue;
    }

    reinterpret_cast<AVI::AVIStreamHeader*>(streamInfo.strhMemorySource->GetData())->dwSuggestedBufferSize = streamInfo.maxDataSize;
  }

  // fix avih
  auto ptrAvihData = reinterpret_cast<AVI::MainAVIHeader*>(avihMemorySource->GetData());
  ptrAvihData->dwSuggestedBufferSize = maxChunkSize;
  ptrAvihData->dwMaxBytesPerSec = 0;
  for (const auto& streamInfo : streamInfoArray) {
    ptrAvihData->dwMaxBytesPerSec += streamInfo.maxBytesPerSec;
  }

  // 完成

  OnFinishAll(riffRoot);

  riffRoot.CreateSource();

  return riffRoot.GetSourceSp();
}
