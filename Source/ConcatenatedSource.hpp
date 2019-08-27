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

  std::vector<PartialSource> mPartialSources;
  std::streamsize mTotalSize;
  std::size_t mLastIndex;

  std::size_t GetIndexFromOffset(std::streamsize offset) const;

public:
  template<typename T>
  ConcatenatedSource(const T& sources) :
    mPartialSources(),
    mTotalSize(0),
    mLastIndex(0)
  {
    const auto numSources = std::size(sources);
    mPartialSources.reserve(numSources);

    std::streamoff offset = 0;
    for (const auto& source : sources) {
      const auto size = source->GetSize();
      assert(size >= 0);
      if (size == 0) {
        continue;
      }
      mPartialSources.push_back(PartialSource{
        offset,
        size,
        source,
      });
      offset += size;
    }
    mTotalSize = offset;
  }

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
};

#endif
