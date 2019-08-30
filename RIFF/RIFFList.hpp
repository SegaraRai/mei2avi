#ifndef ML_RIFFLIST_HPP
#define ML_RIFFLIST_HPP

#include <cstdint>
#include <ios>
#include <memory>

#include "RIFFBase.hpp"
#include "RIFFDirBase.hpp"
#include "../Source/ConcatenatedSource.hpp"
#include "../Source/SourceBase.hpp"


class RIFFList : public RIFFDirBase {
  struct Header {
    std::uint32_t listId;
    std::uint32_t size;
    std::uint32_t chunkId;
  };

  static_assert(sizeof(Header) == 12);

  std::uint32_t mListId;
  std::uint32_t mChunkId;
  Header mHeader;
  std::shared_ptr<ConcatenatedSource> mSource;

protected:
  std::streamsize GetOffsetOf(const RIFFBase* child) const override;

public:
  RIFFList(std::uint32_t listId, std::uint32_t chunkId);

  Type GetType() const override;

  std::streamsize GetSize() const override;
  std::shared_ptr<SourceBase> GetSource() override;
  void CreateSource() override;
};

#endif
