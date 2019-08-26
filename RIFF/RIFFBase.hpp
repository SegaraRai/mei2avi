#ifndef ML_RIFFBASE_HPP
#define ML_RIFFBASE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "../Source/SourceBase.hpp"


class RIFFBase {
protected:
  RIFFBase* parent;

  virtual std::streamsize GetOffsetOf(const RIFFBase* child) const = 0;

public:
  RIFFBase(RIFFBase* parent);
  virtual ~RIFFBase() = default;

  virtual std::streamsize GetOffset() const;
  virtual std::streamsize GetSize() const = 0;
  virtual SourceBase& GetSource() = 0;
  virtual std::shared_ptr<SourceBase> GetSourceSp() = 0;
  virtual void CreateSource();
};

#endif
