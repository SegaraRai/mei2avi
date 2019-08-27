#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ios>

#include "NullSource.hpp"
#include "Util.hpp"


NullSource::NullSource(std::streamsize size) :
  mSize(size)
{}


std::streamsize NullSource::GetSize() const {
  return mSize;
}


void NullSource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
  CheckReadRange(size, offset, mSize);

  if (!size) {
    return;
  }

  std::memset(data, 0, size);
}
