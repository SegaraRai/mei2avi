#include <ios>

#include "RIFFBase.hpp" 


RIFFBase::RIFFBase(RIFFBase* parent) :
  parent(parent)
{}


std::streamsize RIFFBase::GetOffset() const {
  return parent->GetOffset() + parent->GetOffsetOf(this);
}


void RIFFBase::SetParent(RIFFBase* parent) {
  this->parent = parent;
}


void RIFFBase::CreateSource() {
  // do nothing
}
