#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ios>
#include <memory>
#include <utility>

#include "MemorySource.hpp"
#include "SourceBase.hpp"
#include "Util.hpp"


MemorySource::MemorySource(const std::uint8_t* data, std::size_t size) :
  mSize(size),
  mData(mSize ? std::make_unique<std::uint8_t[]>(mSize) : nullptr)
{
  if (mSize) {
    std::memcpy(mData.get(), data, mSize);
  }
}


MemorySource::MemorySource(std::size_t size) :
  mSize(size),
  mData(mSize ? std::make_unique<std::uint8_t[]>(mSize) : nullptr)
{
  if (mSize) {
    std::memset(mData.get(), 0, mSize);
  }
}

MemorySource::MemorySource(std::shared_ptr<std::uint8_t[]> data, std::size_t size) :
  mSize(size),
  mData(data)
{}


MemorySource::MemorySource(SourceBase& source) :
  mSize(static_cast<std::size_t>(source.GetSize())),
  mData(mSize ? std::make_unique<std::uint8_t[]>(mSize) : nullptr)
{
  if (mSize) {
    source.Read(mData.get(), mSize, 0);
  }
}


std::streamsize MemorySource::GetSize() const {
  return static_cast<std::streamsize>(mSize);
}


void MemorySource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
  CheckReadRange(size, offset, mSize);

  if (!size) {
    return;
  }

  std::memcpy(data, mData.get() + offset, size);
}


std::shared_ptr<std::uint8_t[]> MemorySource::GetData() {
  return mData;
}
