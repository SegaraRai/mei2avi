#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

#include "AVI.hpp"
#include "Source/SourceBase.hpp"
#include "RIFF/RIFFChunk.hpp"
#include "RIFF/RIFFList.hpp"
#include "RIFF/RIFFRoot.hpp"


class AVIBuilder {
public:
  static constexpr std::uint32_t DefaultStreamIndexFlags = 0;

  static constexpr std::uint32_t DefaultAvihFlags = AVI::AVIF_HASINDEX;

  static constexpr std::uint_fast32_t DefaultJunkSize = 0;

  using BuilderFlags = unsigned int;
  static constexpr BuilderFlags NoIdx1 = 0x0001;
  static constexpr BuilderFlags NoOdml = 0x0002;
  static constexpr BuilderFlags PrependJunk = 0x0004;

  class AVIStream {
  public:
    struct BlockInfo {
      std::uint_fast32_t size;
      std::uint_fast32_t startTime;
      std::uint_fast32_t duration;
      std::uint32_t indexFlags;
    };

    static constexpr std::uint32_t FourCCauds = AVI::GetFourCC("auds");
    static constexpr std::uint32_t FourCCtxts = AVI::GetFourCC("txts");
    static constexpr std::uint32_t FourCCvids = AVI::GetFourCC("vids");

    static constexpr std::uint32_t FourCCdb = AVI::GetFourCC("\0\0db");
    static constexpr std::uint32_t FourCCdc = AVI::GetFourCC("\0\0dc");
    static constexpr std::uint32_t FourCCtx = AVI::GetFourCC("\0\0tx");
    static constexpr std::uint32_t FourCCwb = AVI::GetFourCC("\0\0wb");

    virtual ~AVIStream() = default;

    virtual std::uint32_t GetFourCC() const = 0;

    virtual std::uint_fast32_t CountStreams() const = 0;
    virtual BlockInfo GetBlockInfo(std::uint_fast32_t index) const = 0;
    virtual std::shared_ptr<SourceBase> GetBlockData(std::uint_fast32_t index) const = 0;

    virtual bool FixSuggestedBufferSize() const;

    virtual AVI::AVIStreamHeader GetStrh() = 0;
    virtual std::shared_ptr<SourceBase> GetStrf() = 0;
    virtual std::shared_ptr<SourceBase> GetStrn();

    virtual void OnFinishListStrl(std::shared_ptr<RIFFList> listStrl);
  };

protected:
  BuilderFlags mBuilderFlags;
  std::vector<std::shared_ptr<AVIStream>> mStreams;
  std::optional<std::size_t> mPrimaryVideoStreamIndex;
  std::uint32_t mAvihFlags;
  std::uint_fast32_t mJunkSize;
  std::shared_ptr<RIFFList> mListInfo;

public:
  void SetAvihFlags(std::uint32_t avihFlags);
  void SetJunkSize(std::uint_fast32_t junkSize);
  void SetListInfo(std::shared_ptr<RIFFList> listInfo);

  void AddStream(std::shared_ptr<AVIStream> stream, bool primaryVideoStream);

  virtual std::uint_fast32_t CountTotalFrames() const;

  virtual std::uint32_t GetAvihMicroSecPerFrame() const;
  virtual std::uint32_t GetAvihWidth() const;
  virtual std::uint32_t GetAvihHeight() const;

  virtual void OnFinishListHdrl(std::shared_ptr<RIFFList> listStrl);
  virtual void OnFinishListMovi(std::shared_ptr<RIFFList> listMovi, bool isAvix);
  virtual void OnFinishRiffAvi(std::shared_ptr<RIFFList> riffAvi, bool isAvix);
  virtual void OnFinishAll(RIFFRoot& riffRoot);

  AVIBuilder(BuilderFlags builderFlags = 0);

  std::shared_ptr<SourceBase> BuildAVI();
};
