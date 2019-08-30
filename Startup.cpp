// comment out the below line to minimize engine initialization 
//#define USE_SAKURAGL_INITIALIZER 1

#include <exception>
#include <iostream>
#include <string_view>

#include <sakura/sakura.h>
#include <sakuragl/sakuragl.h>

#if !USE_SAKURAGL_INITIALIZER
# include <sakuragl/sgl_erisa_lib.h>
# include <sakuragl/sgl_media.h>
# include <sakuragl/sgl2d_image.h>
# include <sakuragl/sgl3d_image.h>
#endif

int xwmain(int argc, wchar_t* argv[]);

using namespace std::literals;


int wmain(int argc, wchar_t* argv[]) {
  // see also
  // - Source/common/sakuragl/sakuragl.cpp
  // - Source/win32/sakuragl/sakuragl_startup_con.cpp

  int ret = 0;

#if USE_SAKURAGL_INITIALIZER
  SakuraGL::Initialize();
#else
  SSystem::Initialize();

  ERISA::sclfInitializeMatrix();
  SakuraGL::SGLImageDecoderManager::Initialzie();
  SakuraGL::SGLImageEncoderManager::Initialzie();
  SakuraGL::SGLAudioDecoderManager::Initialzie();
#endif

  SSystem::SetMemoryAllocationMode(SSystem::mallocModeGlobal);

#ifdef NDEBUG
  try {
    ret = xwmain(argc, argv);
  } catch (std::exception exception) {
    std::cerr << exception.what() << std::endl;
    ret = 11;
  } catch (...) {
    std::wcerr << L"an error occurred"sv << std::endl;
    ret = 11;
  }
#else
  ret = xwmain(argc, argv);
#endif

#if USE_SAKURAGL_INITIALIZER
  SakuraGL::Finalize();
#else
  SakuraGL::SGLImageDecoderManager::Finalize();
  SakuraGL::SGLImageEncoderManager::Finalize();
  SakuraGL::SGLAudioDecoderManager::Finalize();

  SSystem::Finalize();
#endif

  return ret;
}
