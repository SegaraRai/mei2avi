#ifndef ML_RIFFCHUNK_HPP
#define ML_RIFFCHUNK_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "RIFFBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFChunk : public RIFFBase {
  struct Header {
    std::uint32_t chunkId;
    std::uint32_t size;
  };

  static_assert(sizeof(Header) == 8);

  std::uint32_t mChunkId;
  std::shared_ptr<SourceBase> mContentSource;
  std::shared_ptr<ConcatenatedSource> mSource;

  void ChunkCreateSource();

protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFChunk(std::uint32_t chunkId, std::shared_ptr<SourceBase> contentSource);
  RIFFChunk(std::uint32_t chunkId);

  std::streamsize GetSize() const override;
  std::shared_ptr<SourceBase> GetSource() override;

  void SetContentSource(std::shared_ptr<SourceBase> contentSource);
};

#endif
