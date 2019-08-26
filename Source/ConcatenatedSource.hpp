#ifndef ML_CONCATENATEDSOURCE_HPP
#define ML_CONCATENATEDSOURCE_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <ios>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

#include "SourceBase.hpp"


class ConcatenatedSource : public SourceBase {
  struct PartialSource {
    std::streamsize offset;
    std::streamsize size;
    std::shared_ptr<SourceBase> source;
  };

  struct OffsetInfo {
    std::streamsize offset;
    std::streamsize size;
    std::size_t index;
  };


  static constexpr std::streamsize UnusedOffset = -1;

  static constexpr OffsetInfo UnusedOffsetInfo{
    UnusedOffset,
    0,
    std::numeric_limits<std::size_t>::max(),
  };


  std::vector<OffsetInfo> mOffsetBinarySearchTree;    // represent binary tree in the way of heap
  std::vector<PartialSource> mPartialSources;
  std::streamsize mTotalSize;

  void ConstructBinarySearchTree(std::size_t treeIndex, const PartialSource* sources, std::size_t sourceOffset, std::size_t numSources);
  std::size_t GetIndexFromOffset(std::streamsize offset) const;

public:
  template<typename T>
  ConcatenatedSource(const T& sources) :
    mOffsetBinarySearchTree(),
    mPartialSources(),
    mTotalSize(0)
  {
    const auto numSources = std::size(sources);

    // 2^ceil(lb(N)) - 1
    // for convenience of heap index calculation, 0 is not used, so 1 is added
    std::size_t treeSize = 1;
    while (treeSize - 1 < numSources) {
      treeSize *= 2;
    }

    mPartialSources.reserve(numSources);

    std::streamoff offset = 0;
    for (const auto& source : sources) {
      const auto size = source->GetSize();
      assert(size >= 0);
      if (size == 0) {
        continue;
      }
      mPartialSources.push_back({
        offset,
        size,
        source,
      });
      offset += size;
    }
    mTotalSize = offset;

    // construct binary search tree
    mOffsetBinarySearchTree.resize(treeSize, UnusedOffsetInfo);
    ConstructBinarySearchTree(1, mPartialSources.data(), 0, mPartialSources.size());
  }

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
};

#endif
