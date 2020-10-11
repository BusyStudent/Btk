#if !defined(_BTK_IMAGEVIEW_HPP_)
#define _BTK_IMAGEVIEW_HPP_
#include "widget.hpp"
#include "pixels.hpp"
//the image view widget
namespace Btk{
    
    class ImageView:public Widget{
        public:
            ImageView(Window&,int x,int y,int w,int h);
            void draw(Renderer &);
        private:
            
    };
};

#endif // _BTK_IMAGEVIEW_HPP_
