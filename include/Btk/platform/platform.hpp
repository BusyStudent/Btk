#if !defined(_BTK_PLATFORM_HPP_)
#define _BTK_PLATFORM_HPP_
#ifdef _WIN32
    #include "win32.hpp"
    #define BTK_WIN32
#elif defined(__linux) && !defined(__ANDROID__)
    #include "x11.hpp"
    #define BTK_X11
#else
    #error "Unsupport platform"
#endif

namespace Btk{
    namespace Platform{
        #ifdef BTK_WIN32
        using namespace Win32;
        #elif defined(BTK_X11)
        using namespace X11;
        #endif
    }
}

#endif // _BTK_PLATFORM_HPP_
