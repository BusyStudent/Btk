#include "nanovg.h"

//Import Wrap
#include <Btk/utils/template.hpp>

//Import STL
#include <cstdlib>
#include <cmath>
#include <map>

//Import X11
#include <X11/extensions/Xrender.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

namespace{
    struct NVGX11texture{
        ::Picture picture;
        ::Pixmap  pixmap;
        int type;
        int w,h;
    };
    struct NVGX11context{
        ::Display *display = {};
        ::Drawable target = {};
        ::XID      window = {};
          bool     has_shm = {};
        //Init / Cleanup by create and destroy
        ::GC       gc = {};
        ::Picture  win_picture;

        //XRender Info
        ::XRenderPictFormat RGBA32_fmt;
        ::XRenderPictFormat A8_fmt;
        //Members
        std::map<int,NVGX11texture> textures;

        //Method for NVG
        int  create();
        int  create_texture(int type,int w,int h,int imageFlags,const uint8_t* data);
        int  delete_texture(int image);
        int  update_texture(int image,int x,int y,int w,int h,const uint8_t* data);
        int  get_texture_size(int image,int* w,int* h);
        void viewport(float w,float h,float ratio);
        void destory();
        void cancel(){}//N-op
        void flush(){}//N-op
        void fill(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
				  const float* bounds, const NVGpath* paths, int npaths);
        void stroke(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
					float strokeWidth, const NVGpath* paths, int npaths);
        void triangles(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor,
					   const NVGvertex* verts, int nverts, float fringe);
        //Private
        int  alloc_texture();
        void release_texture(NVGX11texture &tex);
        

    };
    template<auto T>
    inline constexpr auto X11wrap = Btk::MemberFunctionWrapper<T>::Invoke;

    NVGcontext *nvgCreateX11(::Display *display,::XID window){
        //Check if supported
        if(!XRenderQueryExtension(display,nullptr,nullptr)){
            //Unsupport
            return nullptr;
        }
        //Check pixels


        NVGX11context *ctxt = new NVGX11context;
        ctxt->has_shm = XShmQueryExtension(display);
        ctxt->display = display;
        ctxt->target = window;
        ctxt->window = window;

        NVGparams params = {};

        params.edgeAntiAlias = 1;
        params.userPtr = ctxt;
        params.renderCreate = X11wrap<&NVGX11context::create>;
        params.renderDelete = X11wrap<&NVGX11context::destory>;
        params.renderCreateTexture = X11wrap<&NVGX11context::create_texture>;
        params.renderDeleteTexture = X11wrap<&NVGX11context::delete_texture>;
        params.renderUpdateTexture = X11wrap<&NVGX11context::update_texture>;
        params.renderGetTextureSize = X11wrap<&NVGX11context::get_texture_size>;
        params.renderViewport = X11wrap<&NVGX11context::viewport>;
        params.renderCancel = X11wrap<&NVGX11context::cancel>;
        params.renderFlush = X11wrap<&NVGX11context::flush>;
        params.renderFill = X11wrap<&NVGX11context::fill>;
        params.renderStroke = X11wrap<&NVGX11context::stroke>;
        params.renderTriangles = X11wrap<&NVGX11context::triangles>;


        NVGcontext *nvg = nvgCreateInternal(&params);
        if(nvg != nullptr){
            return nvg;
        }
        delete ctxt;
        return nullptr;
    }
    void  nvgDeleteX11(NVGcontext *ctxt){
        nvgDeleteInternal(ctxt);
    }
    int  NVGX11context::create(){
        gc = XCreateGC(display,window,0,0);

        A8_fmt     = *XRenderFindStandardFormat(display,PictStandardA8);
        RGBA32_fmt = *XRenderFindStandardFormat(display,PictStandardARGB32);

        //Create window picture
        win_picture = XRenderCreatePicture(
            display,
            window,
            nullptr,
            0,
            nullptr
        );

        return 1;
    }
    void NVGX11context::destory(){
        //Clear all texture
        for(auto iter = textures.begin();iter != textures.end();){
            release_texture(iter->second);
            iter = textures.erase(iter);
        }
        XRenderFreePicture(display,win_picture);
        XFreeGC(display,gc);
    }
    int  NVGX11context::delete_texture(int id){
        auto iter = textures.find(id);
        if(iter == textures.end()){
            return false;
        }
        release_texture(iter->second);
        textures.erase(iter);

        return true;
    }
    void NVGX11context::fill(NVGpaint* paint, NVGcompositeOperationState compositeOperation, NVGscissor* scissor, float fringe,
				  const float* bounds, const NVGpath* paths, int npaths){
        

    }
    int  NVGX11context::create_texture(int type,int w,int h,int imageFlags,const uint8_t* data){
        int id = alloc_texture();
        if(id == -1){
            return -1;
        }
        NVGX11texture &tex = textures[id];

        XRenderPictureAttributes *attr = {};
        XRenderPictFormat *format = {};
        unsigned long attr_mask = {};
        int depth;
        //Process image flags
        if(imageFlags & NVG_IMAGE_REPEATX){
            // attr.repeat = REPEATX;
        }
        //Process type
        if(type == NVG_TEXTURE_RGBA){
            depth = 32;
        }
        else{
            depth = 8;
        }

        tex.type = type;
        tex.w = w;
        tex.h = h;
        tex.pixmap = XCreatePixmap(
            display,
            window,
            w,
            h,
            depth
        );
        tex.picture = XRenderCreatePicture(
            display,
            tex.pixmap,
            format,
            attr_mask,
            attr
        );
        //Update it
        if(data != nullptr){
            update_texture(id,0,0,w,h,data);
        }

        return id;
    }
    int NVGX11context::update_texture(int id,int x,int y,int w,int h,const uint8_t* data){
        auto iter = textures.find(id);
        if(iter == textures.end()){
            return false;
        }
        auto &tex = iter->second;

        XImage *image = XGetImage(
            display,
            tex.pixmap,
            x,
            y,
            w,
            h,
            AllPlanes,
            XYPixmap
        );
        //Put pixls
        if(tex.type == NVG_TEXTURE_RGBA){
            
        }
        else{
            
        }

        XPutImage(
            display,
            tex.pixmap,
            gc,
            image,
            0,
            0,
            x,
            y,
            w,
            h
        );
        XDestroyImage(image);
        return true;
    }
}