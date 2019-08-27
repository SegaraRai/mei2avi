#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "ConcatenatedSource.hpp"
#include "SourceBase.hpp"
#include "Util.hpp"


std::size_t ConcatenatedSource::GetIndexFromOffset(std::streamsize offset) const {
  assert(0 <= offset && offset < mTotalSize);
  assert(!mPartialSources.empty());

  // binary search

  std::size_t leftIndex = 0;
  std::size_t rightIndex = mPartialSources.size() - 1;

  while (leftIndex <= rightIndex) {
    const std::size_t middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
    const auto& partialSource = mPartialSources[middleIndex];
    if (partialSource.offset > offset) {
      // search for left
      rightIndex = middleIndex - 1;
    } else if (partialSource.offset + partialSource.size <= offset) {
      // search for right
      leftIndex = middleIndex + 1;
    } else {
      // found
      return middleIndex;
    }
  }

  throw std::runtime_error("GetIndexFromOffset: index not found");
}


std::streamsize ConcatenatedSource::GetSize() const {
  return mTotalSize;
}


void ConcatenatedSource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
  CheckReadRange(size, offset, mTotalSize);

  const auto firstIndex = GetIndexFromOffset(offset);
  const std::streamsize offsetEnd = offset + size;
  std::streamsize currentOffset = offset;
  std::size_t dataOffset = 0;
  auto itrPartialSource = mPartialSources.cbegin() + firstIndex;
  while (currentOffset != offsetEnd) {
    const auto streamOffset = currentOffset - itrPartialSource->offset;
    const auto readSize = static_cast<std::size_t>(std::min<std::streamsize>(itrPartialSource->size - streamOffset, offsetEnd - currentOffset));
    itrPartialSource->source->Read(data + dataOffset, readSize, streamOffset);
    itrPartialSource++;
    currentOffset += readSize;
    dataOffset += readSize;
  }
}
