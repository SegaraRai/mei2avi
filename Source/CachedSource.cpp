#include <cstddef>
#include <cstdint>
#include <cstring>
#include <ios>
#include <iostream>
#include <memory>

#include "CachedSource.hpp"
#include "SourceBase.hpp"
#include "Util.hpp"


CachedSource::CachedSource(CacheStorage& cacheStorage, std::shared_ptr<SourceBase> source) :
  mPtrCacheStorage(&cacheStorage),
  mSource(source),
  mSize(static_cast<std::size_t>(mSource->GetSize())),
  mCacheId(0)
{}


std::streamsize CachedSource::GetSize() const {
  return mSize;
}


void CachedSource::Read(std::uint8_t* data, std::size_t size, std::streamsize offset) {
  CheckReadRange(size, offset, mSize);

  const auto ptr = mPtrCacheStorage->Get(mCacheId);
  if (ptr) {
    std::memcpy(data, ptr->data.get() + offset, size);
    //std::wcerr << L"cache hit" << std::endl;
    return;
  }
  auto sourceData = std::make_unique<std::uint8_t[]>(mSize);
  mSource->Read(sourceData.get(), mSize, 0);
  std::memcpy(data, sourceData.get() + offset, size);
  mCacheId = mPtrCacheStorage->Add(std::move(sourceData), mSize);
  //std::wcerr << L"cache miss" << std::endl;
  return;
}
