#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "ConcatenatedSource.hpp"
#include "SourceBase.hpp"


void ConcatenatedSource::ConstructBinarySearchTree(std::size_t treeIndex, const PartialSource* sources, std::size_t sourceOffset, std::size_t numSources) {
  // treeIndex starts at 1

  if (numSources == 0) {
    return;
  }

  // the middle element (use the right one if the size is even)
  const auto middleIndex = numSources / 2;

  const auto absoluteMiddleIndex = sourceOffset + middleIndex;
  mOffsetBinarySearchTree[treeIndex] = OffsetInfo{
    sources[absoluteMiddleIndex].offset,
    sources[absoluteMiddleIndex].size,
    absoluteMiddleIndex,
  };

  // left subtree
  ConstructBinarySearchTree(treeIndex * 2, sources, sourceOffset, middleIndex);
  // right subtree
  ConstructBinarySearchTree(treeIndex * 2 + 1, sources, sourceOffset + middleIndex + 1, numSources - middleIndex - 1);
}


std::size_t ConcatenatedSource::GetIndexFromOffset(std::streamsize offset) const {
  assert(0 <= offset && offset < mTotalSize);

  std::size_t treeIndex = 1;
  while (true) {
    assert(treeIndex < mOffsetBinarySearchTree.size());

    const auto& node = mOffsetBinarySearchTree[treeIndex];
    assert(node.offset != UnusedOffset);

    if (node.offset <= offset && offset < node.offset + node.size) {
      // found
      return node.index;
    }
    treeIndex = node.offset > offset ? treeIndex * 2 : treeIndex * 2 + 1;
  }
}


std::streamsize ConcatenatedSource::GetSize() const {
  return mTotalSize;
}


void ConcatenatedSource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
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
