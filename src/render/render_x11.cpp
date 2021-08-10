#include "../build.hpp"

#include <Btk/render.hpp>

#include <X11/Xlib.h>

//TODO

namespace Btk{
    class X11Device:public RendererDevice{
        ::GC gc;
    };
}