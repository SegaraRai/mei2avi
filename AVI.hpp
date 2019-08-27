#ifndef ML_AVI_HPP
#define ML_AVI_HPP

#include <cstdint>

// obtained from https://makiuchi-d.github.io/mksoft/doc/avifileformat.html


namespace AVI {
  constexpr std::uint32_t GetFourCC(const char fourcc[5]) {
    return
      static_cast<std::uint32_t>(fourcc[0]) << 0 |
      static_cast<std::uint32_t>(fourcc[1]) << 8 |
      static_cast<std::uint32_t>(fourcc[2]) << 16 |
      static_cast<std::uint32_t>(fourcc[3]) << 24;
  }


#pragma pack(push, 1)

  struct MainAVIHeader {
    std::uint32_t dwMicroSecPerFrame;    // frame display rate
    std::uint32_t dwMaxBytesPerSec;      // maximum transfer rate
    std::uint32_t dwPaddingGranularity;  // pad to multiple of this size (can be 0)
    std::uint32_t dwFlags;               // flags
    std::uint32_t dwTotalFrames;         // frames in the RIFF-AVI list
    std::uint32_t dwInitialFrames;
    std::uint32_t dwStreams;
    std::uint32_t dwSuggestedBufferSize;
    std::uint32_t dwWidth;
    std::uint32_t dwHeight;
    std::uint32_t dwReserved[4];
  };

  static_assert(sizeof(MainAVIHeader) == 4 * 14);


  struct AVIStreamHeader {
    std::uint32_t fccType;
    std::uint32_t fccHandler;
    std::uint32_t dwFlags;
    std::uint16_t wPriority;
    std::uint16_t wLanguage;
    std::uint32_t dwInitialFrames;
    std::uint32_t dwScale;
    std::uint32_t dwRate;         // dwRate / dwScale == samples/second
    std::uint32_t dwStart;
    std::uint32_t dwLength;       // frame count or sample count
    std::uint32_t dwSuggestedBufferSize;
    std::uint32_t dwQuality;
    std::uint32_t dwSampleSize;
    struct {
      std::uint16_t left;
      std::uint16_t top;
      std::uint16_t right;
      std::uint16_t bottom;
    } rcFrame;
  };

  static_assert(sizeof(AVIStreamHeader) == 4 * 14);


  struct AVIINDEXENTRY {
    std::uint32_t ckid;
    std::uint32_t dwFlags;
    std::uint32_t dwChunkOffset;
    std::uint32_t dwChunkLength;
  };

  static_assert(sizeof(AVIINDEXENTRY) == 4 * 4);


  struct AVISUPERINDEX {
    std::uint16_t wLongsPerEntry;
    std::uint8_t  bIndexSubType;
    std::uint8_t  bIndexType;
    std::uint32_t nEntriesInUse;
    std::uint32_t dwChunkId;
    std::uint32_t dwReserved[3];
  };

  static_assert(sizeof(AVISUPERINDEX) == 4 * 6);

  // flexible array member cannot be used in C++ ...
  struct AVISUPERINDEXENTRY {
    std::uint64_t qwOffset;
    std::uint32_t dwSize;
    std::uint32_t dwDuration;
  };

  static_assert(sizeof(AVISUPERINDEXENTRY) == 4 * 4);


  struct AVISTDINDEX {
    std::uint16_t wLongsPerEntry;
    std::uint8_t  bIndexSubType;
    std::uint8_t  bIndexType;
    std::uint32_t nEntriesInUse;
    std::uint32_t dwChunkId;
    std::uint64_t qwBaseOffset;
    std::uint32_t dwReserved3;
  };

  static_assert(sizeof(AVISTDINDEX) == 4 * 6);

  // flexible array member cannot be used in C++ ...
  struct AVISTDINDEXENTRY {
    std::uint32_t dwoffset;
    std::uint32_t dwSize;
  };

  static_assert(sizeof(AVISTDINDEXENTRY) == 4 * 2);


  //

  struct AVIEXTHEADER {
    std::uint32_t dwGrandFrames;
    std::uint32_t dwFuture[61];
  };

  static_assert(sizeof(AVIEXTHEADER) == 4 * 62);

#pragma pack(pop)


  constexpr std::uint32_t AVIF_HASINDEX          = 0x00000010;
  constexpr std::uint32_t AVIF_MUSTUSEINDEX      = 0x00000020;
  constexpr std::uint32_t AVIF_ISINTERLEAVED     = 0x00000100;
  constexpr std::uint32_t AVIF_TRUSTCKTYPE       = 0x00000800;
  constexpr std::uint32_t AVIF_WASCAPTUREFILE    = 0x00010000;
  constexpr std::uint32_t AVIF_COPYRIGHTED       = 0x00020000;
                                                
  constexpr std::uint32_t AVIIF_INDEX            = 0x00000010;
  constexpr std::uint32_t AVIIF_NO_TIME          = 0x00000100;
                                                
  constexpr std::uint32_t AVISF_DISABLED         = 0x00000001;
  constexpr std::uint32_t AVISF_VIDEO_PALCHANGES = 0x00010000;
  constexpr std::uint32_t AVIIF_LIST             = 0x00000001;
  constexpr std::uint32_t AVIIF_KEYFRAME         = 0x00000010;
}

#endif
