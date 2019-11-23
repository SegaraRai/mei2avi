#ifndef ML_CACHEDSOURCE_HPP
#define ML_CACHEDSOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <optional>

#include "SourceBase.hpp"
#include "../CacheStorage.hpp"


class CachedSource : public SourceBase {
  CacheStorage* mPtrCacheStorage;
  std::shared_ptr<SourceBase> mSource;
  std::size_t mSize;
  std::optional<CacheStorage::Id> mCacheId;

public:
  CachedSource(CacheStorage& cacheStorage, std::shared_ptr<SourceBase> source);

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
};

#endif
