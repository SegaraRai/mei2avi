#ifndef ML_CACHESTORAGE_HPP
#define ML_CACHESTORAGE_HPP

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>


class CacheStorage {
public:
  using Id = std::size_t;

  struct CacheData {
    Id id;
    std::size_t size;
    std::unique_ptr<std::uint8_t[]> data;
  };

private:
  using TimeCount = unsigned long;

  struct CacheInfo {
    Id id;
    std::size_t size;
    TimeCount lastUsed;
  };

  TimeCount mTimeCount;
  Id mLastId;
  std::size_t mTotalSize;
  std::size_t mMaxStorageSize;
  std::size_t mMaxStorageData;
  std::unordered_map<Id, CacheData> mCacheDataMap;
  std::vector<CacheInfo> mCacheInfoHeap;

  static bool CompareCacheInfo(const CacheInfo& a, const CacheInfo& b);

public:
  CacheStorage(std::size_t maxStorageSize, std::size_t maxStorageData);

  Id Remove();
  Id Add(std::unique_ptr<std::uint8_t[]>&& data, std::size_t size);
  Id Add(const std::uint8_t* data, std::size_t size);
  const CacheData* Get(Id id);
};

#endif
