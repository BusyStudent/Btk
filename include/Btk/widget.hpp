#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_

namespace Btk{
    class Renderer;
    class Window;
    class Widget{
        public:
            //some method
            virtual ~Widget();
            virtual void draw(Renderer &render) = 0;
        protected:
            Window *win;
    };
};

#endif // _BTK_WIDGET_HPP_
