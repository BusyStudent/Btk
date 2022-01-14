#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/container.hpp>
#include <Btk/exception.hpp>

#include <Btk/platform/winutils.hpp>
namespace Btk{
    using namespace WinUtils;
    struct EmbedWindow::_Internal{
        NativeHandle win;
        NativeHandle helper;
        SDL_Window *helper_sdl;

        #ifndef NDEBUG
        Painter helper_painter;
        Painter win_painter;
        #endif
    };
}