#include <cassert>
#include <ios>

#include "RIFFBase.hpp" 
#include "RIFFDirBase.hpp" 


RIFFBase::RIFFBase() :
  parent(nullptr)
{}


std::streamsize RIFFBase::GetOffset() const {
  return parent->GetOffset() + parent->GetOffsetOf(this);
}


void RIFFBase::SetParent(RIFFDirBase* parent) {
  assert(!this->parent);
  assert(parent);
  this->parent = parent;
}


void RIFFBase::CreateSource() {
  // do nothing
}
