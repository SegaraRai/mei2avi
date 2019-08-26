#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "CacheStorage.hpp"


bool CacheStorage::CompareCacheInfo(const CacheInfo& a, const CacheInfo& b) {
  return a.lastUsed > b.lastUsed;
}


CacheStorage::CacheStorage(std::size_t maxStorageSize, std::size_t maxStorageData) :
  mTimeCount(0),
  mLastId(1),
  mTotalSize(0),
  mMaxStorageSize(maxStorageSize),
  mMaxStorageData(maxStorageData),
  mCacheDataMap(),
  mCacheInfoHeap()
{}


CacheStorage::Id CacheStorage::Remove() {
  if (mCacheInfoHeap.empty()) {
    throw std::runtime_error("CacheStorage: no data in cache storage");
  }

  std::pop_heap(mCacheInfoHeap.begin(), mCacheInfoHeap.end(), CacheStorage::CompareCacheInfo);
  const auto cacheInfo = *(mCacheInfoHeap.end() - 1);
  mCacheInfoHeap.pop_back();
  mCacheDataMap.erase(cacheInfo.id);
  mTotalSize -= cacheInfo.size;
  return cacheInfo.id;
}


CacheStorage::Id CacheStorage::Add(std::unique_ptr<std::uint8_t[]>&& data, std::size_t size) {
  if (size > mMaxStorageSize) {
    throw std::runtime_error("CacheStorage: data too large");
  }

  while (mTotalSize + size > mMaxStorageSize || mCacheDataMap.size() + 1 > mMaxStorageData) {
    Remove();
  }

  do {
    mLastId++;
  } while (mCacheDataMap.count(mLastId));

  const auto id = mLastId;

  mCacheDataMap.emplace(id, CacheData{
    id,
    size,
    std::move(data),
  });

  mCacheInfoHeap.push_back(CacheInfo{
    id,
    size,
    mTimeCount,
  });

  std::push_heap(mCacheInfoHeap.begin(), mCacheInfoHeap.end(), CacheStorage::CompareCacheInfo);

  mTotalSize += size;

  mTimeCount++;

  return id;
}


CacheStorage::Id CacheStorage::Add(const std::uint8_t* data, std::size_t size) {
  auto upData = std::make_unique<std::uint8_t[]>(size);
  std::memcpy(upData.get(), data, size);
  return Add(std::move(upData), size);
}


const CacheStorage::CacheData* CacheStorage::Get(Id id) {
  auto itrCahceInfo = std::find_if(mCacheInfoHeap.begin(), mCacheInfoHeap.end(), [id] (const CacheInfo& cacheInfo) {
    return cacheInfo.id == id;
  });

  if (itrCahceInfo == mCacheInfoHeap.end()) {
    return nullptr;
  }

  itrCahceInfo->lastUsed = mTimeCount;

  std::make_heap(mCacheInfoHeap.begin(), mCacheInfoHeap.end(), CacheStorage::CompareCacheInfo);

  mTimeCount++;
  return &mCacheDataMap.at(id);
}
