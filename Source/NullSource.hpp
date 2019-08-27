#ifndef ML_NULLSOURCE_HPP
#define ML_NULLSOURCE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>

#include "SourceBase.hpp"


class NullSource : public SourceBase {
  std::streamsize mSize;

public:
  NullSource(std::streamsize size);

  std::streamsize GetSize() const override;
  void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) override;
};

#endif
