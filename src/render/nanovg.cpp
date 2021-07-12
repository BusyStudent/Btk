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
    void Renderer::flush(){
        nvg_ctxt->params.renderFlush(nvg_ctxt->params.userPtr);
    }
    //Default device operations
    void RendererDevice::begin_frame(Context ctxt,float w,float h,float ratio){
        nvgBeginFrame(ctxt,w,h,ratio);
    }
    void RendererDevice::end_frame(Context ctxt){
        nvgEndFrame(ctxt);
    }
    void RendererDevice::cancel_frame(Context ctxt){
        nvgCancelFrame(ctxt);
    }
    TextureID RendererDevice::create_texture(Context ctxt,int w,int h,TextureFlags flags,const void *p){
        return nvgCreateImageRGBA(ctxt,w,h,int(flags),static_cast<const unsigned char*>(p));
    }
    void RendererDevice::update_texture(Context ctxt,TextureID id,const Rect *r,const void *pixels){
        Rect rect = {0,0,0,0};

        if(r == nullptr){
            nvgImageSize(ctxt,id,&rect.w,&rect.h);
        }
        else{
            //Copy it
            rect = *r;
        }
        //Call 
        ctxt->params.renderUpdateTexture(
            ctxt->params.userPtr,
            id,
            rect.x,
            rect.y,
            rect.w,
            rect.h,
            static_cast<const unsigned char*>(pixels)
        );
    }
    void RendererDevice::destroy_texture(Context ctxt,TextureID id){
        nvgDeleteImage(ctxt,id);
    }
}