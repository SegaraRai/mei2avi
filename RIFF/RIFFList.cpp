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
  std::streamsize offset = sizeof(Header);
  auto itrChildren = mChildren.cbegin();
  while (itrChildren->get() != child) {
    offset += (*itrChildren)->GetSize();
    itrChildren++;
  }
  return offset;
}


RIFFList::RIFFList(RIFFBase* parent, std::uint32_t listId, std::uint32_t chunkId) :
  RIFFBase(parent),
  mListId(listId),
  mChunkId(chunkId),
  mHeader{
    listId,
    0,
    chunkId,
  },
  mSource()
{}


std::streamsize RIFFList::GetSize() const {
  std::streamsize size = 0;
  for (const auto& child : mChildren) {
    size += child->GetSize();
  }
  return size + sizeof(Header);
}


SourceBase& RIFFList::GetSource() {
  if (!mSource) {
    throw std::runtime_error("LIST: call CreateSource before GetSource");
  }
  return *mSource;
}


std::shared_ptr<SourceBase> RIFFList::GetSourceSp() {
  if (!mSource) {
    throw std::runtime_error("LIST: call CreateSource before GetSourceSp");
  }
  return mSource;
}


void RIFFList::CreateSource() {
  std::streamsize childrenSize = 0;
  for (const auto& child : mChildren) {
    child->CreateSource();
    childrenSize += child->GetSize();
  }
  if (childrenSize + 4 > std::numeric_limits<decltype(Header::size)>::max()) {
    throw std::runtime_error("LIST: children too large");
  }
  mHeader.size = static_cast<std::uint32_t>(childrenSize + 4);

  std::vector<std::shared_ptr<SourceBase>> sources;
  sources.reserve(mChildren.size() + 1);
  sources.emplace_back(std::make_shared<MemorySource>(reinterpret_cast<const std::uint8_t*>(&mHeader), sizeof(mHeader)));
  for (const auto& child : mChildren) {
    sources.emplace_back(child->GetSourceSp());
  }
  mSource = std::make_shared<ConcatenatedSource>(sources);
}


std::size_t RIFFList::CountChildren() const {
  return mChildren.size();
}


RIFFBase* RIFFList::GetChild(std::size_t index) {
  return mChildren[index].get();
}


const RIFFBase* RIFFList::GetChild(std::size_t index) const {
  return mChildren[index].get();
}


void RIFFList::AddChild(std::shared_ptr<RIFFBase> child) {
  mChildren.push_back(child);
}
