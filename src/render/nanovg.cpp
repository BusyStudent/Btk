#include "../build.hpp"

#include <Btk/font/font.hpp>
#include <Btk/render.hpp>
#include <Btk/font.hpp>

extern "C"{
    #define FONS_USE_FREETYPE
    #define NVG_NO_STB
    #include "../libs/nanovg.h"
    #include "../libs/nanovg.c"
    #include "../libs/fontstash.h"
}
namespace Btk{
    Font Renderer::cur_font(){
        int idx = nvg__getState(nvg_ctxt)->fontId;
        if(idx == FONS_INVALID){
            throwRuntimeError("Invaid font");
        }
        auto i = fontsGetFaceByID(nvg_ctxt->fs,idx);
        if(i == nullptr){
            throwRuntimeError("Invaid font");
        }
        auto p = static_cast<Ft::Font*>(i);
        p->ref();
        return p;
    }
}