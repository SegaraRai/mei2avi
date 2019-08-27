#ifndef ML_RIFFROOT_HPP
#define ML_RIFFROOT_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <vector>

#include "RIFFBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFRoot : public RIFFBase {
  std::vector<std::shared_ptr<RIFFBase>> mChildren;
  std::shared_ptr<ConcatenatedSource> mSource;

protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFRoot();

  std::streamsize GetOffset() const override;
  std::streamsize GetSize() const override;
  std::shared_ptr<SourceBase> GetSource() override;
  void SetParent(RIFFBase* parent) override;
  void CreateSource() override;

  std::size_t CountChildren() const;
  RIFFBase* GetChild(std::size_t index);
  const RIFFBase* GetChild(std::size_t index) const;
  void AddChild(std::shared_ptr<RIFFBase> child);
};

#endif
