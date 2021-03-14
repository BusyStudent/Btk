#include "./build.hpp"

#include <Btk/impl/scope.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/rwops.hpp>
#include <Btk/font.hpp>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>

#include <cstdarg>

#include "libs/nanovg.h"

#define UNPACK_COLOR(C) C.r,C.g,C.b,C.a
#define BTK_NULLCHECK(VAL) if(VAL == nullptr){return;}

namespace Btk{
    void Renderer::done(){
        nvgEndFrame(nvg_ctxt);
        swap_buffer();
    }
    //create texture from pixbuf
    Texture Renderer::create_from(const PixBuf &pixbuf){
        if(pixbuf.empty()){
            throw RuntimeError("The pixbuf is empty");
        }
        //Convert the pixels
        if(pixbuf->format->format != SDL_PIXELFORMAT_RGBA32){
            return create_from(pixbuf.convert(SDL_PIXELFORMAT_RGBA32));
        }
        if(SDL_MUSTLOCK(pixbuf.get())){
            SDL_LockSurface(pixbuf.get());
        }
        int t = nvgCreateImageRGBA(
            nvg_ctxt,
            pixbuf->w,
            pixbuf->h,
            0,
            static_cast<const Uint8*>(pixbuf->pixels)
        );
        if(SDL_MUSTLOCK(pixbuf.get())){
            SDL_UnlockSurface(pixbuf.get());
        }
        return Texture(t,this);
    }
    Texture Renderer::load(std::string_view fname){
        return create_from(PixBuf::FromFile(fname));
    }
    Texture Renderer::load(RWops &rwops){
        return create_from(PixBuf::FromRWops(rwops));
    }
    int  Renderer::copy(const Texture &texture,const Rect *_src,const Rect *_dst){
        //make pattern
        SDL_Rect dst;//Dst
        SDL_Rect src;//Dst
        int w,h;
        if(_dst == nullptr){
            SDL_GetWindowSize(window,&dst.w,&dst.h);
            dst.x = 0;
            dst.y = 0;
        }
        else{
            dst = *_dst;
        }
        if(_src == nullptr){
            src.x = 0;
            src.y = 0;
            auto size = texture.size();
            src.w = size.w;
            src.h = size.h;
        }
        else{
            src = *_src;
        }
        //Check what is the max
        //float radio = 0;

        w = std::max(src.w,dst.w);
        h = std::max(src.h,dst.h);

        auto paint = nvgImagePattern(
            nvg_ctxt,
            src.x + dst.x,
            src.y + dst.y,
            w,
            h,
            0.0f/180.0f*NVG_PI,
            texture.get(),
            1.0f
        );
        nvgBeginPath(nvg_ctxt);
        nvgRect(nvg_ctxt,dst.x,dst.y,w,h);
        nvgFillPaint(nvg_ctxt,paint);
        nvgFill(nvg_ctxt);
        return 0;
    }
    int  Renderer::copy(const PixBuf &pixbuf,const Rect *src,const Rect *dst){
        auto texture = create_from(pixbuf);
        return copy(texture,src,dst);
    }
}
namespace Btk{
    //NVG Method
    void Renderer::stroke(){
        nvgStroke(nvg_ctxt);
    }
    void Renderer::stroke_width(float size){
        nvgStrokeWidth(nvg_ctxt,size);
    }
    void Renderer::stroke_color(Color c){
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
    }
    void Renderer::show_path_caches(){
        nvgDebugDumpPathCache(nvg_ctxt);
    }
    void Renderer::move_to(float x,float y){
        nvgMoveTo(nvg_ctxt,x,y);
    }
    void Renderer::line_to(float x,float y){
        nvgLineTo(nvg_ctxt,x,y);
    }
    void Renderer::begin_path(){
        nvgBeginPath(nvg_ctxt);
    }
    void Renderer::close_path(){
        nvgClosePath(nvg_ctxt);
    }
}
namespace Btk{
    Texture::~Texture(){
        clear();
    }
    Size Texture::size() const{
        Size size;
        nvgImageSize(render->nvg_ctxt,image,&size.w,&size.h);
        return size;
    }
    //delete texture
    void Texture::clear(){
        if(empty()){
            return;
        }
        render->free_texture(image);
        image = 0;
        render = nullptr;        
    }
    Texture &Texture::operator =(Texture &&t){
        if(&t == this){
            return *this;
        }
        clear();
        render = t.render;
        image  = t.image;

        t.render = nullptr;
        t.image = 0;
        return *this;
    }
}
//Dreaptched functions
namespace Btk{
    //Draw a rounded box directly
    int Renderer::rounded_box(const Rect &r,int rad,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgRoundedRect(nvg_ctxt,r.x,r.y,r.w,r.h,rad);
        nvgFillColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgFill(nvg_ctxt);
        return 0;
    }
    int Renderer::rounded_rect(const Rect &r,int rad,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgRoundedRect(nvg_ctxt,r.x,r.y,r.w,r.h,rad);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
    int Renderer::box(const Rect &r,Color c){
        nvgBeginPath(nvg_ctxt);

        nvgRect(nvg_ctxt,r.x,r.y,r.w,r.h);
        nvgFillColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgFill(nvg_ctxt);
        return 0;
    }
    int Renderer::rect(const Rect &r,Color c){
        nvgBeginPath(nvg_ctxt);

        nvgRect(nvg_ctxt,r.x,r.y,r.w,r.h);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
    int  Renderer::line(int x,int y,int x2,int y2,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgMoveTo(nvg_ctxt,x,y);
        nvgLineTo(nvg_ctxt,x2,y2);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
    //cliprect
    int  Renderer::set_cliprect(const Rect *rect){
        if(rect == nullptr){
            nvgResetScissor(nvg_ctxt);
            return 0;
        }
        if(rect->empty()){
            nvgResetScissor(nvg_ctxt);
            return 0;
        }
        cliprect = *rect;
        nvgScissor(
            nvg_ctxt,
            cliprect.x,
            cliprect.y,
            cliprect.w,
            cliprect.h
        );
        return 0;
    }
    Rect Renderer::get_cliprect(){
        return cliprect;
    }
}