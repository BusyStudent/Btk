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
        //Draw something
        void line(int x1,int y1,int x2,int y2);
        void aaline(int x1,int y1,int x2,int y2);
        //fill a rect
        void fill_rect(const SDL_Rect &);
        //draw a rect
        void draw_rect(const SDL_Rect &);
        //some alias
        inline void rect(const SDL_Rect &r){
            fill_rect(r);
        }
        inline void box(const SDL_Rect &r){
            draw_rect(r);
        }
        void start();//Start for rendering
        void done();//Finished rendering
        SDL_Rect current;//Current Postions
        SDL_Color color;//Draw color
        SDL_Renderer *render;
    };
};


#endif // _BTKIMPL_RENDER_HPP_
