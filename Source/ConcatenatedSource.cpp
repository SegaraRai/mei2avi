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
  assert(!mPieces.empty());

  // cache

  assert(mLastUsedIndex < mPieces.size());
  const auto& lastUsedPiece = mPieces[mLastUsedIndex];
  if (lastUsedPiece.offset <= offset && offset < lastUsedPiece.offset + lastUsedPiece.size) {
    //std::wcerr << L"cache hit!" << std::endl;
    return mLastUsedIndex;
  }
  //std::wcerr << L"cache miss" << std::endl;

  // binary search

  std::size_t leftIndex = 0;
  std::size_t rightIndex = mPieces.size() - 1;

  while (leftIndex <= rightIndex) {
    const std::size_t middleIndex = leftIndex + (rightIndex - leftIndex) / 2;
    const auto& piece = mPieces[middleIndex];
    if (piece.offset > offset) {
      // left
      rightIndex = middleIndex - 1;
    } else if (piece.offset + piece.size <= offset) {
      // right
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
    auto& piece = mPieces[index];
    const auto streamOffset = currentOffset - piece.offset;
    const auto readSize = static_cast<std::size_t>(std::min<std::streamsize>(piece.size - streamOffset, offsetEnd - currentOffset));
    piece.source->Read(data + dataOffset, readSize, streamOffset);
    currentOffset += readSize;
    dataOffset += readSize;
    index++;
  }

  // cache
  assert(!mPieces.empty());
  mLastUsedIndex = index - 1;
  if (index < mPieces.size() && currentOffset == mPieces[index].offset) {
    mLastUsedIndex++;
  }
}
