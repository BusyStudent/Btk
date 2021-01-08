#if !defined(_BTK_RENDERER_HPP_)
#define _BTK_RENDERER_HPP_
#include "pixels.hpp"
#include "rect.hpp"
#include "defs.hpp"
struct SDL_Window;
namespace Btk{
    /**
     * @brief Abstruct Renderer
     * 
     */
    class BTKAPI Renderer{
        public:
            Renderer() = default;
            Renderer(const Renderer &) = delete;
        public:
            //Some virtual draw methods
            virtual int pixel(int x,int y) = 0;
            virtual int pixels(const Vec2 *vec2,int n) = 0;
            virtual int line(int x1,int y1,int x2,int y2) = 0;
            virtual int lines(const Vec2 *vec2,int n) = 0;
            virtual int rects(const Rect *rect,int n) = 0;
            virtual int rect(const Rect &rect) = 0;
            virtual int box(const Rect &rect) = 0;
            virtual int boxs(const Rect *rect,int n) = 0;
            virtual int set_color(Color color) = 0;

            virtual Rect get_cliprect();
            /**
             * @brief Set the renderer cliprect
             * 
             * @param r The cliprect point(nullptr to reset it)
             * @return 0 on sucuess 
             */
            virtual int  set_cliprect(const Rect *r = nullptr);
            /**
             * @brief Set the renderer cliprect
             * 
             * @param r The cliprect
             * @return 0 on sucuess 
             */
            virtual int  set_cliprect(const Rect &r){
                return set_cliprect(&r);
            }
        public:
            /**
             * @brief Draw AAline
             * 
             * @param x1 The line's x1
             * @param y1 The line's y1
             * @param x2 The line's x2
             * @param y2 The line's y2
             * @param c The line's color
             * @return int 
             */
            int aaline(int x1,int y1,int x2,int y2,Color c);
            int rounded_box(const Rect &,int rad,Color c);
            int rounded_rect(const Rect &,int rad,Color c);
            //virtual int control(int code,...);
            virtual ~Renderer(){};
        public:
            //Texture
            virtual void *impl_create_texture(Uint32 pixfmt,int access,int w,int h) = 0;
    };
    typedef Renderer *(*RendererCreateFn)(SDL_Window*);
    
    BTKAPI void RegisterRenderer(RendererCreateFn);
};


#endif // _BTK_RENDERER_HPP_
