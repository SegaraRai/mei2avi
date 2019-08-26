#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "RIFFRoot.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


std::streamsize RIFFRoot::GetOffsetOf(const RIFFBase* child) const {
  std::streamsize offset = 0;
  auto itrChildren = mChildren.cbegin();
  while (itrChildren->get() != child) {
    offset += (*itrChildren)->GetSize();
    itrChildren++;
  }
  return offset;
}


RIFFRoot::RIFFRoot() :
  RIFFBase(nullptr),
  mSource()
{}


std::streamsize RIFFRoot::GetOffset() const {
  return 0;
}


std::streamsize RIFFRoot::GetSize() const {
  std::streamsize size = 0;
  for (const auto& child : mChildren) {
    size += child->GetSize();
  }
  return size;
}


SourceBase& RIFFRoot::GetSource() {
  if (!mSource) {
    throw std::runtime_error("ROOT: call CreateSource before GetSource");
  }
  return *mSource;
}


std::shared_ptr<SourceBase> RIFFRoot::GetSourceSp() {
  if (!mSource) {
    throw std::runtime_error("ROOT: call CreateSource before GetSourceSp");
  }
  return mSource;
}


void RIFFRoot::CreateSource() {
  std::vector<std::shared_ptr<SourceBase>> sources;
  sources.reserve(mChildren.size());
  for (const auto& child : mChildren) {
    child->CreateSource();
    sources.emplace_back(child->GetSourceSp());
  }
  mSource = std::make_shared<ConcatenatedSource>(sources);
}


std::size_t RIFFRoot::CountChildren() const {
  return mChildren.size();
}


RIFFBase* RIFFRoot::GetChild(std::size_t index) {
  return mChildren[index].get();
}


const RIFFBase* RIFFRoot::GetChild(std::size_t index) const {
  return mChildren[index].get();
}


void RIFFRoot::AddChild(std::shared_ptr<RIFFBase> child) {
  mChildren.push_back(child);
}
