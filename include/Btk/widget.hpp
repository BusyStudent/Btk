#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "signal/signal.hpp"
#include "rect.hpp"
namespace Btk{
    class Renderer;
    class Window;
    class Widget:public HasSlots{
        public:
            //some method
            Widget() = default;
            Widget(const Widget &) = delete;
            virtual ~Widget();
            virtual void draw(Renderer &render) = 0;
        protected:
            bool is_hided;
            Rect pos;//Widget pos
            Window *win;
        friend class  Window;
        friend class  Layout;
        friend struct WindowImpl;
    };
    class Container:public Widget{
        public:
            Container();
            ~Container();
            //add widgets
            template<class T,class ...Args>
            T& add(Args &&...args){
                T *ptr = new T(
                    std::forward<Args>(args)...
                );
                widgets_list.push_back(ptr);
                return *ptr;
            }
        protected:
            std::list<Widget*> widgets_list;
    };
};

#endif // _BTK_WIDGET_HPP_
