#if !defined(_BTKIMPL_RENDER_HPP_)
#define _BTKIMPL_RENDER_HPP_
#include <SDL2/SDL_render.h>
namespace Btk{
    class Surface;
    class Texture;
    struct Renderer{

        Renderer(SDL_Renderer *ren = nullptr):render(ren){};
        ~Renderer();
        //Draw something
        void line(int x1,int y1,int x2,int y2,SDL_Color c);
        void aaline(int x1,int y1,int x2,int y2,SDL_Color c);
        //fill a rect
        void fill_rect(const SDL_Rect &,SDL_Color c);
        //draw a rect
        void draw_rect(const SDL_Rect &,SDL_Color c);
        void rounded_box(const SDL_Rect &,int rad,SDL_Color c);
        void rounded_rect(const SDL_Rect &,int rad,SDL_Color c);
        //some alias
        inline void rect(const SDL_Rect &r,SDL_Color c){
            fill_rect(r,c);
        }
        inline void box(const SDL_Rect &r,SDL_Color c){
            draw_rect(r,c);
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
        void start(SDL_Color bgcolor);//Start for rendering
        void done();//Finished rendering
        //texture methods
        //create texture
        Texture create_from(const Surface &surf);
        Surface dump_texture(const Texture &);//dump texture to surface

        SDL_Renderer *render;
    };
    
};


#endif // _BTKIMPL_RENDER_HPP_
