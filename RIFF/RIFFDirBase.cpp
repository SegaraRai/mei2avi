#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <vector>

#include "RIFFDirBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


std::streamsize RIFFDirBase::GetChildOffsetOf(const RIFFBase* child) const {
  std::streamsize offset = 0;
  auto itrChildren = children.cbegin();
  while (itrChildren->get() != child) {
    offset += (*itrChildren)->GetSize();
    itrChildren++;
  }
  return offset;
}


RIFFDirBase::RIFFDirBase() :
  RIFFBase(),
  children(),
  contentSource()
{}


std::streamsize RIFFDirBase::GetContentSize() const {
  std::streamsize size = 0;
  for (const auto& child : children) {
    size += child->GetSize();
  }
  return size;
}


void RIFFDirBase::CreateContentSource() {
  std::vector<std::shared_ptr<SourceBase>> sources;
  sources.reserve(children.size());
  for (const auto& child : children) {
    child->CreateSource();
    sources.emplace_back(child->GetSource());
  }
  contentSource = std::make_shared<ConcatenatedSource>(sources);
}


std::size_t RIFFDirBase::CountChildren() const {
  return children.size();
}


RIFFBase* RIFFDirBase::GetChild(std::size_t index) {
  return children[index].get();
}


const RIFFBase* RIFFDirBase::GetChild(std::size_t index) const {
  return children[index].get();
}


void RIFFDirBase::AppendChild(std::shared_ptr<RIFFBase> child) {
  child->SetParent(this);
  children.push_back(child);
}


void RIFFDirBase::PrependChild(std::shared_ptr<RIFFBase> child) {
  child->SetParent(this);
  children.push_front(child);
}
