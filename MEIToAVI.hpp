#ifndef ML_MEITOAVi_HPP
#define ML_MEITOAVi_HPP

#include "CacheStorage.hpp"
#include "RIFF/RIFFRoot.hpp"
#include "Source/SourceBase.hpp"

#include <memory>
#include <string>

#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>


class MEIToAVI {
public:
  static constexpr unsigned int NoMessage   = 0x0001;
  static constexpr unsigned int NoAudio     = 0x0002;
  static constexpr unsigned int NoAlpha     = 0x0004;
  static constexpr unsigned int NoApproxFPS = 0x0008;

  struct Options {
    unsigned int flags;
    std::size_t cacheStorageSize;
    std::size_t cacheStorageLimit;
    std::uint_fast32_t audioBlockSamples;
    std::uint_fast32_t junkChunkSize;
  };

private:
  class FrameImageSource : public SourceBase {
    ERISA::SGLMovieFilePlayer* mPtrMovieFilePlayer;
    std::size_t mFrameIndex;
    std::size_t mSize;

  public:
    FrameImageSource(ERISA::SGLMovieFilePlayer& movieFilePlayer, std::uint_fast32_t frameIndex);

    std::streamsize GetSize() const override;
    void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
  };

  CacheStorage mCacheStorage;
  SSystem::SFile mFile;
  ERISA::SGLMovieFilePlayer mMovieFilePlayer;
  RIFFRoot mRiffRoot;

public:
  MEIToAVI(const std::wstring& filePath, const Options& options);

  SourceBase& GetSource();
};

#endif
