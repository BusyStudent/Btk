#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_

namespace Btk{
    struct WidgetImpl;
    class Widget{
        public:
            //some method
            ~Widget();
            void draw();
        protected:
            WidgetImpl *pimpl;
    };
};

#endif // _BTK_WIDGET_HPP_
