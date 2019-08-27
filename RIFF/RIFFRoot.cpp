#include <ios>
#include <memory>
#include <stdexcept>

#include "RIFFRoot.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


std::streamsize RIFFRoot::GetOffsetOf(const RIFFBase* child) const {
  return GetChildOffsetOf(child);
}


RIFFRoot::RIFFRoot() :
  RIFFDirBase()
{}


std::streamsize RIFFRoot::GetOffset() const {
  return 0;
}


std::streamsize RIFFRoot::GetSize() const {
  return GetContentSize();
}


std::shared_ptr<SourceBase> RIFFRoot::GetSource() {
  if (!contentSource) {
    throw std::runtime_error("ROOT: call CreateSource before GetSource");
  }
  return contentSource;
}


void RIFFRoot::SetParent(RIFFBase* parent) {
  throw std::logic_error("ROOT: SetParent is not available");
}


void RIFFRoot::CreateSource() {
  CreateContentSource();
}
