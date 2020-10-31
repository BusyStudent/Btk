#if !defined(_BTK_IMAGEVIEW_HPP_)
#define _BTK_IMAGEVIEW_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
//the image view widget
namespace Btk{
    class ImageView:public Widget{
        public:
            ImageView();
            ImageView(Window&,int x,int y,int w,int h);
            ~ImageView();
            //set image
            void set_image(const PixBuf &buf);
            //ref image
            void ref_image(PixBuf &buf);
            //redraw self
            void draw();
            //called from parent widget
            void draw(Renderer &);
        private:
            PixBuf pixelbuf;
            Texture texture;
            Rect image_rect;
    };
};

#endif // _BTK_IMAGEVIEW_HPP_
