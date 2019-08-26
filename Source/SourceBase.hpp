#ifndef ML_SOURCEBASE_HPP
#define ML_SOURCEBASE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>


class SourceBase {
public:
  virtual ~SourceBase() = default;

  virtual std::streamsize GetSize() const = 0;
  virtual void Read(std::uint8_t* data, std::size_t size, std::streamsize offset) = 0;
};

#endif
