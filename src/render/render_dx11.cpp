#include "../build.hpp"

#include <d3d11.h>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>

extern "C"{
    #define NANOVG_D3D11_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_d3d11.h"
}
//TODO Finish it
#define DEVICE(PTR) static_cast<ID3D11Device *>(PTR)
namespace Btk{
    Renderer::Renderer(SDL_Window *win){
        //Create d3d context
        device = nullptr;
        nvg_ctxt = nvgCreateD3D11(DEVICE(device),NVG_STENCIL_STROKES | NVG_ANTIALIAS);
        if(nvg_ctxt == nullptr){
            //HandleErrr
            //throwRendererError();
        }
    }
    Renderer::~Renderer(){
        destroy();
    }
    void Renderer::destroy(){
        if(nvg_ctxt != nullptr){
            nvgDeleteD3D11(nvg_ctxt);
            //Release the d3d device
            DEVICE(device)->Release();
            nvg_ctxt = nvg_ctxt;
            device = nullptr;
        }
    }
    RendererBackend Renderer::backend() const{
        return RendererBackend::Dx11;
    }
}