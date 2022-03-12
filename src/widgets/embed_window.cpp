#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/container.hpp>
#include <Btk/exception.hpp>

#include <Btk/platform/winutils.hpp>

#if 0

namespace{
    using namespace Btk::WinUtils;

    #if BTK_X11
    static ::Display *sdl_xdisplay = nullptr;
    ::Display *get_display(){
        if(sdl_xdisplay == nullptr){
            SDL_Window *win = SDL_CreateWindow(
                nullptr,
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                100,
                100,
                SDL_WINDOW_HIDDEN
            );
            auto handle = GetHandleFrom(win);
            sdl_xdisplay = handle.display;
            SDL_DestroyWindow(win);
        }
        return sdl_xdisplay;
    }
    void release_helper(NativeHandle helper){
        XDestroyWindow(helper.display,helper.window);
    }
    #endif
}

namespace Btk{
    struct EmbedWindow::_Internal{
        NativeHandle win;
        NativeHandle helper;

        #ifndef NDEBUG
        Painter helper_painter;
        Painter win_painter;
        #endif

    };
}

#endif