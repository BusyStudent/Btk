#if !defined(_BTKIMPL_RENDER_HPP_)
#define _BTKIMPL_RENDER_HPP_
#include <SDL2/SDL_render.h>
namespace Btk{
    struct RendererImpl{
        RendererImpl(SDL_Window *win);
        ~RendererImpl();
        void set_color(SDL_Color c){
            color.r = c.r;
            color.g = c.g;
            color.b = c.b;
            color.a = c.a;
        }
        void start();//Start for rendering
        void done();//Finished rendering
        SDL_Rect current;//Current Postions
        SDL_Color color;//Draw color
        SDL_Renderer *render;
    };
};


#endif // _BTKIMPL_RENDER_HPP_
