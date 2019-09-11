#define NOMINMAX

#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <ios>
#include <iostream>
#include <io.h>
#include <fcntl.h>

#include "MEIToAVI.hpp"

using namespace std::literals;


namespace {
  constexpr std::size_t CacheStorageSize = std::numeric_limits<std::size_t>::max();
  constexpr std::size_t CacheStorageLimit = 2;
  constexpr std::size_t DefaultAudioBlockSamples = 0;
  constexpr std::size_t DefaultJunkSize = 4096;
  constexpr std::size_t DefaultBufferSize = 64 * 1024;


  int ShowUsage(const wchar_t* program) {
    std::wcerr << L"mei2avi v0.2.0"sv << std::endl;
    std::wcerr << L"Copyright (c) 2019 SegaraRai"sv << std::endl;
    std::wcerr << std::endl;
    std::wcerr << L"usage: "sv << program << L" [-quiet] [-noaudio] [-noalpha] [-orgfps] [-ablock sample] [-junksize size] [-bufsize size] infile outfile"sv << std::endl;
    std::wcerr << std::endl;
    std::wcerr << L"-quiet      suppress messages"sv << std::endl;
    std::wcerr << L"-noaudio    skip decoding audio"sv << std::endl;
    std::wcerr << L"-noalpha    assume that the source has no alpha channel"sv << std::endl;
    std::wcerr << L"-orgfps     use original frame rate"sv << std::endl;
    std::wcerr << L"-ablock     set the number of samples for each audio block (default: "sv << DefaultAudioBlockSamples << L", set 0 to calculate automatically)"sv << std::endl;
    std::wcerr << L"-junksize   set the size of JUNK chunk (default: "sv << DefaultJunkSize << L", set 0 to disable JUNK chunk)"sv << std::endl;
    std::wcerr << L"-bufsize    set buffer size for output (default: "sv << DefaultBufferSize << L")"sv << std::endl;
    std::wcerr << std::endl;
    std::wcerr << L"set outfile to \"-\" to output to stdout"sv << std::endl;
    std::wcerr << std::endl;
    std::wcerr << L"This program makes use of EntisGLS version 4s.05." << std::endl;
    std::wcerr << std::endl;
    std::wcerr << L"EntisGLS version 4s.05"sv << std::endl;
    std::wcerr << L"Copyright (c) 1998-2014 Leshade Entis, Entis soft."sv << std::endl;
    return 1;
  }
}


int xwmain(int argc, wchar_t* argv[]) {
  MEIToAVI::Options options{
    0,
    CacheStorageSize,
    CacheStorageLimit,
    DefaultAudioBlockSamples,
    DefaultJunkSize,
  };

  std::size_t bufferSize = DefaultBufferSize;

  int argIndex = 1;
  while (argIndex < argc) {
    const std::wstring arg(argv[argIndex]);
    argIndex++;

    if (arg == L"-quiet"sv) {
      options.flags |= MEIToAVI::NoMessage;
      continue;
    }

    if (arg == L"-noaudio"sv) {
      options.flags |= MEIToAVI::NoAudio;
      continue;
    }

    if (arg == L"-noalpha"sv) {
      options.flags |= MEIToAVI::NoAlpha;
      continue;
    }

    if (arg == L"-orgfps"sv) {
      options.flags |= MEIToAVI::NoApproxFPS;
      continue;
    }

    if (arg == L"-ablock"sv) {
      const auto argSamples = std::stoll(argv[argIndex++]);
      if (argSamples < 0) {
        std::wcerr << L"sample must be greater than or equal to 0" << std::endl;
        return 2;
      }
      options.audioBlockSamples = static_cast<std::uint_fast32_t>(argSamples);
      continue;
    }

    if (arg == L"-junksize"sv) {
      const auto argJunkChunkSize = std::stoll(argv[argIndex++]);
      if (argJunkChunkSize < 0) {
        std::wcerr << L"size must be greater than or equal to 0" << std::endl;
        return 2;
      }
      options.junkChunkSize = static_cast<std::uint_fast32_t>(argJunkChunkSize);
      continue;
    }

    if (arg == L"-bufsize"sv) {
      const auto argBufferSize = std::stoll(argv[argIndex++]);
      if (argBufferSize < 1) {
        std::wcerr << L"size must be greater than 0" << std::endl;
        return 2;
      }
      bufferSize = static_cast<std::size_t>(argBufferSize);
      continue;
    }

    argIndex--;

    break;
  };

  if (argIndex + 2 != argc) {
    return ShowUsage(argv[0]);
  }

  const std::wstring inFile(argv[argIndex++]);
  const std::wstring outFile(argv[argIndex++]);

  const bool useStdOut = outFile == L"-"sv;

  std::ofstream ofs;
  if (useStdOut) {
    if (_setmode(_fileno(stdout), _O_BINARY) == -1) {
      throw std::runtime_error("_setmode failed"s);
    }
  } else {
    ofs.exceptions(std::ios::failbit | std::ios::badbit);
    ofs.open(outFile, std::ios::binary);
  }

  {
    MEIToAVI meiToAvi(inFile, options);
    auto& source = meiToAvi.GetSource();
    const std::streamsize totalSize = source.GetSize();

    if (!(options.flags & MEIToAVI::NoMessage)) {
      std::wcerr << L"[info] avi size = "sv << totalSize << L" bytes"sv << std::endl;
    }

    auto buffer = std::make_unique<std::uint8_t[]>(bufferSize);
    std::streamsize offset = 0;

    std::ostream* ptrOs = useStdOut ? &std::cout : &ofs;

    while (offset != totalSize) {
      const auto readSize = static_cast<std::size_t>(std::min<std::streamsize>(bufferSize, totalSize - offset));
      source.Read(buffer.get(), readSize, offset);
      ptrOs->write(reinterpret_cast<const char*>(buffer.get()), readSize);
      offset += readSize;
    }

    ptrOs->flush();
  }

  if (!useStdOut) {
    ofs.close();
  }

  return 0;
}
