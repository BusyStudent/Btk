#if !defined(_BTK_IMPL_APPLICATION_HPP_)
#define _BTK_IMPL_APPLICATION_HPP_

#include <list>

#include "../defs.hpp"
#include "../window.hpp"
#include "window.hpp"

#define BTK_X11_DECL(X)
#define BTK_WIN32_DECL(X)
#define BTK_ANDROID_DECL(X)

#ifdef __gnu_linux__
    #include "../platform/dbus.hpp"
    #undef BTK_X11_DECL
    #define BTK_X11_DECL(X) X
#elif defined(_WIN32)
    #define NOMINMAX
    #include <windows.h>
    #undef BTK_WIN32_DECL
    #define BTK_WIN32_DECL(X) X
#else
    #warning "Unsupport platform"
#endif
namespace Btk{
    struct BTKHIDDEN ApplicationImpl{
        ApplicationImpl();
        ApplicationImpl(const ApplicationImpl &) = delete;
        ~ApplicationImpl();

        BTK_X11_DECL(DBus::Connection dbus_con);
        BTK_X11_DECL(DBus::Error dbus_err);

        BTK_WIN32_DECL(HWND app_handle = nullptr);

        std::list<Window> managed_windows;
        
    };
}
#endif // _BTK_IMPL_APPLICATION_HPP_
