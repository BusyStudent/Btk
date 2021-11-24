#include "../build.hpp"

#include <Btk/utils/template.hpp>
#include <Btk/render.hpp>

#include <algorithm>
#include <queue>
#include <map>

#include <X11/Xlib.h>

//TODO

#include "../libs/nanovg.h"

namespace Btk{
    struct X11Texture{
        XImage image;
        int id;
    };
    struct X11Context{
        X11Context(::Display *d,::Drawable draw);
        
        ::Display *xdisplay;
        ::Drawable xtarget;
        ::GC       xgc;
        
        //For nvg interface
        int  create();
        int  create_texture(int type,int w,int h,int imageFlags,const Uint8* data);
        int  delete_texture(int image);
        int  update_texture(int image,int x,int y,int w,int h,const Uint8* data);
        int  get_texture_size(int image,int* w,int* h);
        void viewport(float w,float h,float ratio);
        void destory();
        void cancel();
        void flush();
        void fill(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
				  const float* bounds, const NVGpath* paths, int npaths);
        void stroke(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
					float strokeWidth, const NVGpath* paths, int npaths);
        void triangles(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor,
					   const NVGvertex* verts, int nverts, float fringe);

        std::map<TextureID,X11Texture> texs_map;

        template<auto T>
        static constexpr auto Wrapper = MemberFunctionWrapper<T>::Invoke;
        static NVGcontext *Create(::Display *d,::Drawable draw);
        static void        Delete(NVGcontext *ctxt);
    };
    void X11Context::destory(){
        delete this;
    }
    NVGcontext *X11Context::Create(::Display *d,::Drawable draw){
        auto ctxt = new X11Context(d,draw);
        NVGparams params;
        params.userPtr = ctxt;
        params.renderCreate = Wrapper<&X11Context::create>;
        params.renderDelete = Wrapper<&X11Context::destory>;
        params.renderCreateTexture = Wrapper<&X11Context::create_texture>;
        params.renderDeleteTexture = Wrapper<&X11Context::delete_texture>;
        params.renderUpdateTexture = Wrapper<&X11Context::update_texture>;
        params.renderGetTextureSize = Wrapper<&X11Context::get_texture_size>;
        params.renderViewport = Wrapper<&X11Context::viewport>;
        params.renderCancel = Wrapper<&X11Context::cancel>;
        params.renderFlush = Wrapper<&X11Context::flush>;
        params.renderFill = Wrapper<&X11Context::fill>;
        params.renderStroke = Wrapper<&X11Context::stroke>;
        params.renderTriangles = Wrapper<&X11Context::triangles>;

        return nvgCreateInternal(&params);
    }
    void      X11Context::Delete(NVGcontext *ctxt){
        nvgDeleteInternal(ctxt);
    }
    class X11Device:public RendererDevice{
        ::GC gc;
    };
}