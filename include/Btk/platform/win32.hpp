#if !defined(_BTK_PLATFORM_WIN32)
#define _BTK_PLATFORM_WIN32
#include <windows.h>
#include "../defs.hpp"
namespace Btk{
namespace Win32{
    BTKAPI void Init();
    BTKAPI void Quit();
}
}
#endif // _BTK_PLATFORM_WIN32
