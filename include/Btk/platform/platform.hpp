#if !defined(_BTK_PLATFORM_HPP_)
#define _BTK_PLATFORM_HPP_
#include "../defs.hpp"
#if BTK_WIN32
    #include "win32.hpp"
#elif BTK_X11
    #include "x11.hpp"
#else
    #error "Unsupport platform"
#endif

namespace Btk{
    namespace Platform{
        #if BTK_WIN32
        using namespace Win32;
        #elif BTK_X11
        using namespace X11;
        #endif
    }
}

#endif // _BTK_PLATFORM_HPP_
