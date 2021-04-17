#if !defined(_BTK_IMAGEVIEW_HPP_)
#define _BTK_IMAGEVIEW_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
#include "defs.hpp"
//the image view widget
namespace Btk{
    class BTKAPI ImageView:public Widget{
        public:
            ImageView();
            ImageView(int x,int y,int w,int h);
            ~ImageView();
            //set image
            void set_image(const PixBuf &buf);
            //ref image
            void ref_image(PixBuf &buf);
            //called from parent widget
            void draw(Renderer &);
            //Clip this image
            void set_clip(const Rect &r);

            PixBuf &image(){
                return pixelbuf;
            }
            const PixBuf &image() const{
                return pixelbuf;
            }
            void set_draw_boarder(bool flags = true);
        private:
            PixBuf pixelbuf;
            Texture texture;
            Rect image_rect;
            Color boarder_color = {208,208,208,255};
            Color bg_color;
            bool draw_borader = false;//< Should we draw the boarder
            bool dirty = false;
            
            TextureFlags tex_flags = TextureFlags::Linear;//< The flags of the texture
    };
    /**
     * @brief Gif player
     * 
     */
    class BTKAPI GifView:public Widget{
        public:
            GifView();
            GifView(int x,int y,int w,int h);
            ~GifView();

            bool handle(Event &);
            void draw(Renderer &);

            void set_image(GifImage &&image);
        private:
            GifImage gifimage;
            PixBuf frame;
            Texture texture;
            size_t cur_frame;//current frame
            bool delete_texture = false;
    };
}

#endif // _BTK_IMAGEVIEW_HPP_
