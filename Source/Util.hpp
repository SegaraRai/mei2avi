#ifndef ML_SOURCEUTIL_HPP
#define ML_SOURCEUTIL_HPP

#include <cassert>
#include <cstddef>
#include <ios>


inline void CheckReadRange(std::size_t size, std::streamsize offset, std::streamsize maxSize) {
  assert(offset >= 0);
  assert(maxSize >= 0);

  assert(static_cast<std::streamsize>(size) <= maxSize);
  assert(offset <= maxSize);
  assert(static_cast<std::streamsize>(size) + offset <= maxSize);
}

#endif
