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
        using Win32::Init;
        using Win32::Quit;
        using Win32::HandleSysMsg;
        #elif defined(BTK_X11)
        using X11::Init;
        using X11::Quit;
        using X11::HandleSysMsg;
        using X11::MessageBox;
        #endif
    }
}

#endif // _BTK_PLATFORM_HPP_
