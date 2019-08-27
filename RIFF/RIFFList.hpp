#ifndef ML_RIFFLIST_HPP
#define ML_RIFFLIST_HPP

#include <cstddef>
#include <cstdint>
#include <ios>
#include <memory>
#include <vector>

#include "RIFFBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFList : public RIFFBase {
  struct Header {
    std::uint32_t listId;
    std::uint32_t size;
    std::uint32_t chunkId;
  };

  std::vector<std::shared_ptr<RIFFBase>> mChildren;
  std::uint32_t mListId;
  std::uint32_t mChunkId;
  Header mHeader;
  std::shared_ptr<ConcatenatedSource> mSource;

protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFList(std::uint32_t listId, std::uint32_t chunkId);

  std::streamsize GetSize() const override;
  SourceBase& GetSource() override;
  std::shared_ptr<SourceBase> GetSourceSp() override;
  void CreateSource() override;

  std::size_t CountChildren() const;
  RIFFBase* GetChild(std::size_t index);
  const RIFFBase* GetChild(std::size_t index) const;
  void AddChild(std::shared_ptr<RIFFBase> child);
};

#endif
