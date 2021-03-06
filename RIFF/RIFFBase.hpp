#ifndef ML_RIFFBASE_HPP
#define ML_RIFFBASE_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>

#include "../Source/SourceBase.hpp"


class RIFFDirBase;

class RIFFBase {
public:
  enum class Type {
    Chunk,
    List,
    Root,
  };

protected:
  RIFFDirBase* parent;

public:
  RIFFBase();
  virtual ~RIFFBase() = default;

  virtual Type GetType() const = 0;

  virtual std::streamsize GetOffset() const;
  virtual std::streamsize GetSize() const = 0;
  virtual std::shared_ptr<SourceBase> GetSource() = 0;

  virtual void SetParent(RIFFDirBase* parent);

  virtual void CreateSource();
};

#endif
