#include <array>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "RIFFList.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/MemorySource.hpp"
#include "../Source/SourceBase.hpp"


std::streamsize RIFFList::GetOffsetOf(const RIFFBase* child) const {
  return GetContentOffsetOf(child) + sizeof(Header);
}


RIFFList::RIFFList(std::uint32_t listId, std::uint32_t chunkId) :
  RIFFDirBase(),
  mListId(listId),
  mChunkId(chunkId),
  mHeader{
    listId,
    0,
    chunkId,
  },
  mSource()
{}


RIFFBase::Type RIFFList::GetType() const {
  return Type::List;
}


std::streamsize RIFFList::GetSize() const {
  return GetContentSize() + sizeof(Header);
}


std::shared_ptr<SourceBase> RIFFList::GetSource() {
  if (!mSource) {
    throw std::runtime_error("RIFFList: call CreateSource before GetSource");
  }
  return mSource;
}


void RIFFList::CreateSource() {
  CreateContentSource();

  std::streamsize childrenSize = GetContentSize();
  if (childrenSize + 4 > std::numeric_limits<decltype(Header::size)>::max()) {
    throw std::runtime_error("RIFFList: children too large");
  }
  mHeader.size = static_cast<std::uint32_t>(childrenSize + 4);

  mSource = std::make_shared<ConcatenatedSource>(std::array<std::shared_ptr<SourceBase>, 2>{
    std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&mHeader), sizeof(mHeader)),
    contentSource,
  });
}
