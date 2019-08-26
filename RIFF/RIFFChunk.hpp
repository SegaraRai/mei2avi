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

  std::uint32_t mChunkId;
  std::shared_ptr<SourceBase> mContentSource;
  std::shared_ptr<ConcatenatedSource> mSource;

  void CreateSource();

protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFChunk(RIFFBase* parent, std::uint32_t chunkId, std::shared_ptr<SourceBase> contentSource);
  RIFFChunk(RIFFBase* parent, std::uint32_t chunkId);

  std::streamsize GetSize() const override;
  SourceBase& GetSource() override;
  std::shared_ptr<SourceBase> GetSourceSp() override;

  void SetContentSource(std::shared_ptr<SourceBase> contentSource);
};

#endif
