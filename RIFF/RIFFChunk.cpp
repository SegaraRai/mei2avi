#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>
#include <memory>
#include <stdexcept>

#include "RIFFChunk.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/MemorySource.hpp"
#include "../Source/SourceBase.hpp"


void RIFFChunk::CreateSource() {
  if (mContentSource->GetSize() > std::numeric_limits<std::uint32_t>::max()) {
    throw std::runtime_error("CHUNK: content too large");
  }

  const Header header{
    mChunkId,
    static_cast<std::uint32_t>(mContentSource->GetSize()),
  };

  mSource = std::make_shared<ConcatenatedSource>(std::array<std::shared_ptr<SourceBase>, 2>{
    std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&header), sizeof(header)),
    mContentSource,
  });
}


std::streamsize RIFFChunk::GetOffsetOf(const RIFFBase* child) const {
  throw std::runtime_error("CHUNK: GetOffsetOf is not supported for RIFFChunk");
}


RIFFChunk::RIFFChunk(RIFFBase* parent, std::uint32_t chunkId, std::shared_ptr<SourceBase> contentSource) :
  RIFFBase(parent),
  mChunkId(chunkId),
  mContentSource(contentSource),
  mSource()
{
  CreateSource();
}


RIFFChunk::RIFFChunk(RIFFBase* parent, std::uint32_t chunkId) :
  RIFFChunk(parent, chunkId, std::make_shared<MemorySource>(0))
{}


std::streamsize RIFFChunk::GetSize() const {
  return mSource->GetSize();
}


SourceBase& RIFFChunk::GetSource() {
  return *mSource;
}


std::shared_ptr<SourceBase> RIFFChunk::GetSourceSp() {
  return mSource;
}


void RIFFChunk::SetContentSource(std::shared_ptr<SourceBase> contentSource) {
  mContentSource = contentSource;
  CreateSource();
}
