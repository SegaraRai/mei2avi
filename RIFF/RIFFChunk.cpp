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
#include "../Source/NullSource.hpp"
#include "../Source/SourceBase.hpp"


void RIFFChunk::ChunkCreateSource() {
  static auto paddingSource = std::make_shared<NullSource>(1);

  const auto contentSize = mContentSource->GetSize();

  if (contentSize > std::numeric_limits<std::uint32_t>::max()) {
    throw std::runtime_error("RIFFChunk: content too large");
  }

  const Header header{
    mChunkId,
    static_cast<std::uint32_t>(contentSize),
  };

  std::vector<std::shared_ptr<SourceBase>> sources{
    std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&header), sizeof(header)),
    mContentSource,
  };

  if (contentSize & 1) {
    sources.push_back(paddingSource);
  }

  mSource = std::make_shared<ConcatenatedSource>(sources);
}


RIFFChunk::RIFFChunk(std::uint32_t chunkId, std::shared_ptr<SourceBase> contentSource) :
  RIFFBase(),
  mChunkId(chunkId),
  mContentSource(contentSource),
  mSource()
{
  ChunkCreateSource();
}


RIFFChunk::RIFFChunk(std::uint32_t chunkId) :
  RIFFChunk(chunkId, std::make_shared<MemorySource>(0))
{}


std::streamsize RIFFChunk::GetSize() const {
  return mSource->GetSize();
}


std::shared_ptr<SourceBase> RIFFChunk::GetSource() {
  return mSource;
}


void RIFFChunk::SetContentSource(std::shared_ptr<SourceBase> contentSource) {
  mContentSource = contentSource;
  ChunkCreateSource();
}
