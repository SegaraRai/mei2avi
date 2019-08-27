#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iostream>
#include <memory>

#include "ConcatenatedSource.hpp"
#include "SourceBase.hpp"
#include "Util.hpp"


std::size_t ConcatenatedSource::GetIndexFromOffset(std::streamsize offset) const {
  assert(0 <= offset && offset < mTotalSize);
  assert(!mPartialSources.empty());

  // cache

  assert(mLastIndex < mPartialSources.size());
  const auto& lastPartialSource = mPartialSources[mLastIndex];
  if (lastPartialSource.offset <= offset && offset < lastPartialSource.offset + lastPartialSource.size) {
    //std::wcerr << L"cache hit!" << std::endl;
    return mLastIndex;
  }
  //std::wcerr << L"cache miss" << std::endl;

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
  std::size_t index = firstIndex;
  while (currentOffset != offsetEnd) {
    auto& partialSource = mPartialSources[index];
    const auto streamOffset = currentOffset - partialSource.offset;
    const auto readSize = static_cast<std::size_t>(std::min<std::streamsize>(partialSource.size - streamOffset, offsetEnd - currentOffset));
    partialSource.source->Read(data + dataOffset, readSize, streamOffset);
    currentOffset += readSize;
    dataOffset += readSize;
    index++;
  }

  // cache
  assert(!mPartialSources.empty());
  mLastIndex = index - 1;
  if (index < mPartialSources.size() && currentOffset == mPartialSources[index].offset) {
    mLastIndex++;
  }
}
