#include <Btk/impl/render.hpp>
namespace Btk{
    RendererImpl::RendererImpl(SDL_Window *win){
        render = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
        if(render == nullptr){
            //...
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
} // namespace Btk
