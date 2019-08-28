#include <sakura/sakura.h>
#include <sakuragl/sakuragl.h>


int xwmain(int argc, wchar_t* argv[]);


int wmain(int argc, wchar_t* argv[]) {
  // see EntisGLS4s.05/Source/win32/sakuragl/sakuragl_startup_con.cpp

  int ret = 0;

  SakuraGL::Initialize();

  SSystem::SetMemoryAllocationMode(SSystem::mallocModeGlobal);

  ret = xwmain(argc, argv);

  SakuraGL::Finalize();

  return ret;
}
