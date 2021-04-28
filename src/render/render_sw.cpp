#include "../build.hpp"

//Software render
#include <Btk/render.hpp>
#include <cassert>
extern "C"{
    #define NANOVG_RT_IMPLEMENTATION
    #include "../libs/nanovg.h"
}

namespace Btk{
    Renderer::~Renderer(){
        destroy();
    }
    void Renderer::destroy(){
        if(nvg_ctxt != nullptr){

            nvg_ctxt = nullptr;
        }
    }
}