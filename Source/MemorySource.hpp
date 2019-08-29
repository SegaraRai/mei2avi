#ifndef ML_MEMORYSOURCE_HPP
#define ML_MEMORYSOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "SourceBase.hpp"


class MemorySource : public SourceBase {
  std::size_t mSize;
  std::shared_ptr<std::uint8_t[]> mData;

public:
  MemorySource(const std::uint8_t* data, std::size_t size);
  MemorySource(std::size_t size);
  MemorySource(std::shared_ptr<std::uint8_t[]>&& data, std::size_t size);
  MemorySource(SourceBase& source);

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;

  std::shared_ptr<std::uint8_t[]> GetData();
};

#endif
