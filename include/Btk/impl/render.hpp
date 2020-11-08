#if !defined(_BTKIMPL_RENDER_HPP_)
#define _BTKIMPL_RENDER_HPP_
#include <SDL2/SDL_render.h>
#include "../rect.hpp"
namespace Btk{
    class Surface;
    class Texture;
    struct Renderer{
        //Software render
        Renderer(Surface &);
        Renderer(SDL_Renderer *ren = nullptr):render(ren){};
        ~Renderer();
        //Draw something
        int line(int x1,int y1,int x2,int y2,SDL_Color c);
        int aaline(int x1,int y1,int x2,int y2,SDL_Color c);
        //fill a rect
        int fill_rect(const SDL_Rect &,SDL_Color c);
        //draw a rect
        int draw_rect(const SDL_Rect &,SDL_Color c);
        int rounded_box(const SDL_Rect &,int rad,SDL_Color c);
        int rounded_rect(const SDL_Rect &,int rad,SDL_Color c);
        //some alias
        inline int rect(const SDL_Rect &r,SDL_Color c){
            return fill_rect(r,c);
        }
        inline int box(const SDL_Rect &r,SDL_Color c){
            return draw_rect(r,c);
        }
        //operators
        Renderer &operator =(SDL_Renderer *renderer){
            render = renderer;
            return *this;
        }
        SDL_Renderer* operator *() const noexcept{
            return render;
        }
        bool operator ==(SDL_Renderer *r) const noexcept{
            return render == r;
        }
        //ViewPort
        Rect get_viewport();//Get ViewPort
        int  set_viewport();//Reset View[prt]
        int  set_viewport(const SDL_Rect &r);//Set ViewPort
        //ClipRect
        Rect get_cliprect();
        int  set_cliprect();
        int  set_cliprect(const SDL_Rect &r);
        //copy texture
        int  copy(const Texture &t,const SDL_Rect *src,const SDL_Rect *dst);
        int  start(SDL_Color bgcolor);//Start for rendering
        void done();//Finished rendering
        //texture methods
        //create texture
        Texture create_from(const Surface &surf);
        Surface dump_texture(const Texture &);//dump texture to surface

        SDL_Renderer *render;
    };
    
};


#endif // _BTKIMPL_RENDER_HPP_
