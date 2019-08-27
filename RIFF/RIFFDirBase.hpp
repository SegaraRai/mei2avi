#ifndef ML_RIFFDIRBASE_HPP
#define ML_RIFFDIRBASE_HPP

#include <cstddef>
#include <cstdint>
#include <deque>
#include <ios>
#include <memory>

#include "RIFFBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFDirBase : public RIFFBase {
protected:
  std::deque<std::shared_ptr<RIFFBase>> children;
  std::shared_ptr<ConcatenatedSource> contentSource;

  std::streamsize GetChildOffsetOf(const RIFFBase* child) const;

public:
  RIFFDirBase();

  std::streamsize GetContentSize() const;
  void CreateContentSource();

  std::size_t CountChildren() const;
  RIFFBase* GetChild(std::size_t index);
  const RIFFBase* GetChild(std::size_t index) const;
  void AppendChild(std::shared_ptr<RIFFBase> child);
  void PrependChild(std::shared_ptr<RIFFBase> child);
};

#endif
