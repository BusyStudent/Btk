#include <Btk/impl/render.hpp>
#include <Btk/impl/scope.hpp>
//for Btk_defer
#include <Btk/exception.hpp>
#include <Btk/pixels.hpp>
#ifdef BTK_USE_GFX
    #include "../thirdparty/SDL2_gfxPrimitives.h"
#endif
#define UNPACK_COLOR(C) C.r,C.g,C.b,C.a
namespace Btk{
    Renderer::~Renderer(){
        SDL_DestroyRenderer(render);
    }
    int Renderer::start(SDL_Color bgcolor){
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(bgcolor));
        return SDL_RenderClear(render);
        
    }
    void Renderer::done(){
        SDL_RenderPresent(render);
    }
    //Draw something
    int Renderer::line(int x1,int y1,int x2,int y2,SDL_Color color){
        #ifdef BTK_USE_GFX
        return lineRGBA(render,x1,y1,x2,y2,UNPACK_COLOR(color));
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(bgcolor));
        return SDL_RenderDrawLine(render,x1,y1,x2,y2);
        #endif
    }
    int Renderer::aaline(int x1,int y1,int x2,int y2,SDL_Color color){
        #ifdef BTK_USE_GFX
        return aalineRGBA(render,x1,y1,x2,y2,UNPACK_COLOR(color));
        #else
        //doesnot support
        Renderer::line(x1,y1,x2,y2);
        #endif
    }
    int Renderer::draw_rect(const SDL_Rect &rect,SDL_Color color){
        #ifdef BTK_USE_GFX
        if(not SDL_RectEmpty(&rect)){
            return rectangleRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                UNPACK_COLOR(color)
            );
        }
        else{
            return -1;
        }
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(color));
        return SDL_RenderDrawRect(render,&rect);
        #endif
    }
    int Renderer::fill_rect(const SDL_Rect &rect,SDL_Color color){
        #ifdef BTK_USE_GFX
        if(not SDL_RectEmpty(&rect)){
            return boxRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                UNPACK_COLOR(color)
            );
        }
        else{
            return -1;
        }
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(color));
        return SDL_RenderFillRect(render,&rect);
        #endif
    }
    int Renderer::rounded_rect(const SDL_Rect &rect,int rad,SDL_Color c){
        #ifdef BTK_USE_GFX
        if(not SDL_RectEmpty(&rect)){
            return roundedRectangleRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                rad,
                UNPACK_COLOR(c)
            );
        }
        else{
            return -1;
        }
        #endif
    }
    int Renderer::rounded_box(const SDL_Rect &rect,int rad,SDL_Color c){
        #ifdef BTK_USE_GFX
        if(not SDL_RectEmpty(&rect)){
            return roundedBoxRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                rad,
                UNPACK_COLOR(c)
            );
        }
        return -1;
        #endif
    }
    //texture methods
    int Renderer::copy(const Texture &t,const SDL_Rect *src,const SDL_Rect *dst){
        return SDL_RenderCopy(render,t.texture,src,dst);
    }
    Texture Renderer::create_from(const Surface &surf){
        SDL_Texture *t = SDL_CreateTextureFromSurface(
            render,
            surf.get()
        );
        if(t == nullptr){
            throwSDLError();
        }
        return t;
    }
    Surface Renderer::dump_texture(const Texture &t){
        SDL_PixelFormat *fmt = nullptr;
        void *pixels = nullptr;
        Uint32 ufmt;
        int w,h;
        //cleanup 
        Btk_defer{
            SDL_FreeFormat(fmt);
            SDL_SetRenderTarget(render,nullptr);
            if(pixels != nullptr){
                free(pixels);
            }
        };
        //get fmt w h
        if(SDL_QueryTexture(t.texture,&ufmt,nullptr,&w,&h) == -1){
            throwSDLError();
        }
        if(SDL_SetRenderTarget(render,t.texture) == -1){
            //current target
            throwSDLError();
        }
        //alloc format
        fmt = SDL_AllocFormat(ufmt);
        //alloc buffer
        pixels = malloc(w * h * (fmt->BytesPerPixel));;
        if(pixels == nullptr){
            SDL_OutOfMemory();
            throwSDLError();
        }
        //read pixels
        if(SDL_RenderReadPixels(render,nullptr,ufmt,pixels,w * fmt->BytesPerPixel) == -1){
            //Error
            throwSDLError();
        }
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
            pixels,
            w,
            h,
            fmt->BytesPerPixel,
            fmt->BytesPerPixel * w,
            ufmt
        );
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
    }
} // namespace Btk
