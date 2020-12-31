#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "signal/signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    struct Renderer;
    class  Window;
    class  Event;
    //Attribute for Widget
    struct WidgetAttr{
        bool hide = false;//<Is hide
        bool user_rect = false;//<Using user defined position
        bool container = false;//<Is container
        bool disable = false;//<The widget is disabled?
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
            /**
             * @brief Process event
             * 
             * @return true if widget processed it
             * @return false if widget unprocessed it
             */
            virtual bool handle(Event &);
            bool visible() const noexcept{
                return not attr.hide;
            };
            Vec2 position() const noexcept{
                return {
                    rect.x,
                    rect.y
                };
            };
            Window &master() const noexcept{
                return *win;
            };
            //Set widget rect
            void set_rect(const Rect &rect);
            void set_rect(int x,int y,int w,int h){
                 set_rect({x,y,w,h});
            };
            void set_position(const Vec2 &vec2){
                set_rect(vec2.x,vec2.y,rect.w,rect.h);
            };
            int x() const noexcept{
                return rect.x;
            };
            int y() const noexcept{
                return rect.y;
            };
            int w() const noexcept{
                return rect.w;
            };
            int h() const noexcept{
                return rect.h;
            };
            bool is_enable() const noexcept{
                return not attr.disable;
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
            /**
             * @brief Dispatch Event to each widget
             * 
             * @return true 
             * @return false 
             */
            bool handle(Event &);
        protected:
            std::list<Widget*> widgets_list;
        private:
            Widget *focus_widget = nullptr;//The keyboard focus widget
            Widget *drag_widget = nullptr;//The Dragging event
            Widget *cur_widget = nullptr;//Mouse point widget
            
    };
};

#endif // _BTK_WIDGET_HPP_
