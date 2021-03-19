#include "../build.hpp"

//Software render
#include <Btk/render.hpp>
#include <cassert>
extern "C"{
    #define NANOVG_RT_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_rt.h"
}

namespace Btk{
    Renderer::~Renderer(){
        destroy();
    }
    void Renderer::destroy(){
        if(nvg_ctxt != nullptr){
            nvgDeleteRT(nvg_ctxt);

            nvg_ctxt = nullptr;
        }
    }
}