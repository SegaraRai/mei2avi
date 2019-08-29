#include <cstddef>
#include <cstdint>
#include <ios>

#include "PartialSource.hpp"
#include "SourceBase.hpp"
#include "Util.hpp"


PartialSource::PartialSource(std::shared_ptr<SourceBase> source, std::streamsize offset, std::streamsize size) :
  mSource(source),
  mOffset(0),
  mSize(0)
{
  const auto originalSize = source->GetSize();

  mOffset = offset >= 0 ? offset : originalSize - offset;
  if (mOffset < 0 || mOffset > originalSize) {
    throw std::runtime_error("PartialSource: offset out of range");
  }

  mSize = size == MaxSize
    ? originalSize - mOffset
    : size >= 0
      ? size
      : originalSize - mOffset - size;
  if (mSize < 0 || mSize > originalSize) {
    throw std::runtime_error("PartialSource: size out of range");
  }

  if (mOffset + mSize > originalSize) {
    throw std::runtime_error("PartialSource: offset + size out of range");
  }
}


std::streamsize PartialSource::GetSize() const {
  return mSize;
}


void PartialSource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
  CheckReadRange(size, offset, mSize);
  mSource->Read(data, size, offset + mOffset);
  return;
}
