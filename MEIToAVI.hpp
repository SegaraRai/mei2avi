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
  CacheStorage mCacheStorage;
  SSystem::SFile mFile;
  ERISA::SGLMovieFilePlayer mMovieFilePlayer;
  std::shared_ptr<SourceBase> mAvi;

public:
  MEIToAVI(const std::wstring& filePath, const Options& options);

  SourceBase& GetSource();
};

#endif
