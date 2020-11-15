#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "signal/signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    class Renderer;
    class Window;
    class Event;
    //Attribute for Widget
    struct WidgetAttr{
        bool hide = false;//Is hide
        bool user_rect = false;//Using user defined position
        bool container = false;//Is container
    };
    //Alignment
    enum class Align:unsigned int{
        Center,

        //Vertical Alignment
        Top,
        Buttom,
        //Horizontal Alignment
        Right,
        Left
    };
    class BTKAPI Widget:public HasSlots{
        public:
            /**
             * @brief Construct a new Widget object
             * @note All data will be inited to 0
             */
            Widget():attr(),rect(
                0,0,0,0
            ),win(nullptr){};

            Widget(const Widget &) = delete;
            virtual ~Widget();
            virtual void draw(Renderer &render) = 0;
            virtual bool handle(Event &);//Process Event return true for accept event
            bool visible() const noexcept{
                return not attr.hide;
            }
            Vec2 position() const noexcept{
                return {
                    rect.x,
                    rect.y
                };
            };
            Window &master() const noexcept{
                return *win;
            };
        protected:
            WidgetAttr attr;
            Rect rect;//Widget rect
            Window *win;
        friend class  Window;
        friend class  Layout;
        friend struct WindowImpl;
    };
    class BTKAPI Container:public Widget{
        public:
            Container();
            ~Container();
            //add widgets template
            template<class T,class ...Args>
            T& add(Args &&...args){
                T *ptr = new T(
                    *win,
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
