#include "../build.hpp"

#include <Btk/utils/template.hpp>
#include <Btk/render.hpp>

#include <algorithm>
#include <queue>
#include <map>

#include <X11/Xlib.h>

//TODO

#include "../libs/nanovg.h"
#include "../libs/nanovg_x11.hpp"

namespace Btk{
    class X11Device:public RendererDevice{
        ::Display *display;
        ::Drawable  window;
        
        Context create_context() override;
        void    destroy_context(Context) override;
    };
    auto X11Device::create_context() -> Context{
        return nvgCreateX11(display,window);
    }
    void X11Device::destroy_context(Context ctxt){
        nvgDeleteX11(ctxt);
    }
}