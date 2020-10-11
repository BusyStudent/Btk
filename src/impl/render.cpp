#include <Btk/impl/render.hpp>
#include <Btk/exception.hpp>
#ifdef BTK_USE_GFX
    #include "../thirdparty/SDL2_gfxPrimitives.h"
#endif
#define UNPACK_COLOR(C) C.r,C.g,C.b,C.a
namespace Btk{
    Renderer::~Renderer(){
        SDL_DestroyRenderer(render);
    }
    void Renderer::start(SDL_Color bgcolor){
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(bgcolor));
        SDL_RenderClear(render);
        
    }
    void Renderer::done(){
        SDL_RenderPresent(render);
    }
    //Draw something
    void Renderer::line(int x1,int y1,int x2,int y2,SDL_Color color){
        #ifdef BTK_USE_GFX
        lineRGBA(render,x1,y1,x2,y2,UNPACK_COLOR(color));
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(bgcolor));
        SDL_RenderDrawLine(render,x1,y1,x2,y2);
        #endif
    }
    void Renderer::aaline(int x1,int y1,int x2,int y2,SDL_Color color){
        #ifdef BTK_USE_GFX
        aalineRGBA(render,x1,y1,x2,y2,UNPACK_COLOR(color));
        #else
        //doesnot support
        Renderer::line(x1,y1,x2,y2);
        #endif
    }
    void Renderer::draw_rect(const SDL_Rect &rect,SDL_Color color){
        #ifdef BTK_USE_GFX
        rectangleRGBA(
            render,
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            UNPACK_COLOR(color)
        );
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(color));
        SDL_RenderDrawRect(render,&rect);
        #endif
    }
    void Renderer::fill_rect(const SDL_Rect &rect,SDL_Color color){
        #ifdef BTK_USE_GFX
        boxRGBA(
            render,
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            UNPACK_COLOR(color)
        );
        #else
        SDL_SetRenderDrawColor(render,UNPACK_COLOR(color));
        SDL_RenderFillRect(render,&rect);
        #endif
    }
} // namespace Btk
