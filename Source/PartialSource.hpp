#ifndef ML_PARTIALSOURCE_HPP
#define ML_PARTIALSOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <limits>

#include "SourceBase.hpp"


class PartialSource : public SourceBase {
  static constexpr std::streamsize MaxSize = std::numeric_limits<std::streamsize>::max();

  std::shared_ptr<SourceBase> mSource;
  std::streamsize mOffset;
  std::streamsize mSize;

public:
  PartialSource(std::shared_ptr<SourceBase> source, std::streamsize offset = 0, std::streamsize size = MaxSize);

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
};

#endif
