#if !defined(_BTK_IMAGEVIEW_HPP_)
#define _BTK_IMAGEVIEW_HPP_
#include "widget.hpp"
//the image view widget
namespace Btk{
    struct ImageViewImpl;
    class Window;
    class ImageView:public Widget{
        public:
            ImageView(Window&,int x,int y,int w,int h);
        private:
            //Get impl;
            ImageViewImpl *self() const noexcept{
                return reinterpret_cast<ImageViewImpl*>(pimpl);
            };
    };
};

#endif // _BTK_IMAGEVIEW_HPP_
