#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "signal/signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    class Renderer;
    class Window;
    //Attribute for Widget
    struct WidgetAttr{
        bool hide = false;//Is hide
        bool user_pos = false;//Using user defined position
    };
    class BTKAPI Widget:public HasSlots{
        public:
            //some method
            Widget() = default;
            Widget(const Widget &) = delete;
            virtual ~Widget();
            virtual void draw(Renderer &render) = 0;
            bool visible() const noexcept{
                return not attr.hide;
            }
        protected:
            WidgetAttr attr;
            Rect pos;//Widget pos
            Window *win;
        friend class  Window;
        friend class  Layout;
        friend struct WindowImpl;
    };
    class BTKAPI Container:public Widget{
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
