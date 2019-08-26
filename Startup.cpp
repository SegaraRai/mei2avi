#include <sakura/sakura.h>
#include <sakuragl/sakuragl.h>


int xwmain(int argc, wchar_t* argv[]);


int wmain(int argc, wchar_t* argv[]) {
  int ret = 0;

  SakuraGL::Initialize();

  SSystem::SetMemoryAllocationMode(SSystem::mallocModeGlobal);

  ret = xwmain(argc, argv);

  SakuraGL::Finalize();

  return ret;
}
