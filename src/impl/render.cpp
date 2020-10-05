#include <Btk/impl/render.hpp>
#include <Btk/exception.hpp>
#ifdef BTK_USE_GFX
    #include <SDL2/SDL2_gfxPrimitives.h>
#endif
namespace Btk{
    RendererImpl::RendererImpl(SDL_Window *win){
        render = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
        if(render == nullptr){
            //...
            throwSDLError();
        }
        SDL_SetRenderDrawBlendMode(render,SDL_BLENDMODE_BLEND);
        current.h = 0;
        current.w = 0;
        current.x = 0;
        current.y = 0;

        color.a = 0;
        color.r = 0;
        color.g = 0;
        color.b = 0;
    }
    RendererImpl::~RendererImpl(){
        SDL_DestroyRenderer(render);
    }
    void RendererImpl::start(){
        SDL_SetRenderDrawColor(render,color.r,color.g,color.b,color.a);
        SDL_RenderClear(render);
        
    }
    void RendererImpl::done(){
        SDL_RenderPresent(render);
    }
    //Draw something
    void RendererImpl::line(int x1,int y1,int x2,int y2){
        #ifdef BTK_USE_GFX
        lineRGBA(render,x1,y1,x2,y2,color.r,color.g,color.b,color.a);
        #else
        SDL_SetRenderDrawColor(render,color.r,color.g,color.b,color.a);
        SDL_RenderDrawLine(render,x1,y1,x2,y2);
        #endif
    }
    void RendererImpl::aaline(int x1,int y1,int x2,int y2){
        #ifdef BTK_USE_GFX
        aalineRGBA(render,x1,y1,x2,y2,color.r,color.g,color.b,color.a);
        #else
        //doesnot support
        RendererImpl::line(x1,y1,x2,y2);
        #endif
    }
    void RendererImpl::draw_rect(const SDL_Rect &rect){
        #ifdef BTK_USE_GFX
        rectangleRGBA(
            render,
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            color.r,
            color.g,
            color.b,
            color.a
        );
        #else
        SDL_SetRenderDrawColor(render,color.r,color.g,color.b,color.a);
        SDL_RenderDrawRect(render,&rect);
        #endif
    }
    void RendererImpl::fill_rect(const SDL_Rect &rect){
        #ifdef BTK_USE_GFX
        boxRGBA(
            render,
            rect.x,
            rect.y,
            rect.x + rect.w,
            rect.y + rect.h,
            color.r,
            color.g,
            color.b,
            color.a
        );
        #else
        SDL_SetRenderDrawColor(render,color.r,color.g,color.b,color.a);
        SDL_RenderFillRect(render,&rect);
        #endif
    }
} // namespace Btk
