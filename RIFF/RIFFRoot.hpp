#ifndef ML_RIFFROOT_HPP
#define ML_RIFFROOT_HPP

#include <ios>
#include <memory>

#include "RIFFBase.hpp"
#include "RIFFDirBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFRoot : public RIFFDirBase {
protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFRoot();

  std::streamsize GetOffset() const override;
  std::streamsize GetSize() const override;
  std::shared_ptr<SourceBase> GetSource() override;
  void SetParent(RIFFDirBase* parent) override;
  void CreateSource() override;
};

#endif
