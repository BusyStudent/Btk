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
            ImageView(Window&);
            ImageView(Window&,int x,int y,int w,int h);
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
        private:
            PixBuf pixelbuf;
            Texture texture;
            Rect image_rect;
    };
};

#endif // _BTK_IMAGEVIEW_HPP_
