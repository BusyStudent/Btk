#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "signal/signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    struct Renderer;
    class  Window;
    //Event forward decl
    class  Event;
    struct KeyEvent;
    struct MouseEvent;
    struct MotionEvent;
    struct TextInputEvent;

    enum class FocusPolicy{
        None,
        KeyBoard,
        Click,
        Whell
    };
    //Attribute for Widget
    struct WidgetAttr{
        bool hide = false;//<Is hide
        bool user_rect = false;//<Using user defined position
        bool container = false;//<Is container
        bool disable = false;//<The widget is disabled?
        FocusPolicy focus = FocusPolicy::None;//<Default the widget couldnot get focus
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
    enum class Orientation:unsigned int{
        Vertical = 0,
        Horizontal = 1,
        V = Vertical,
        H = Horizontal
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
        friend class  EventDispatcher;
    };
    /**
     * @brief A helper of dispatch event in widget
     * 
     */
    class EventDispatcher{
        public:
            using container_type = std::list<Widget*>;
            
            EventDispatcher(container_type &w):
                widgets(w){};
            EventDispatcher(const EventDispatcher &) = delete;
            ~EventDispatcher() = default;
        public:
            //Process Event
            bool handle_click(MouseEvent   &);
            bool handle_motion(MotionEvent &);
            bool handle_keyboard(KeyEvent  &);
            bool handle_textinput(TextInputEvent &);
            /**
             * @brief Generic to dispatch event to widgets
             * 
             * @return true 
             * @return false 
             */
            bool handle(Event &);
        private:
            //A helper for set the current focus widget
            void set_focus_widget(Widget *);

            container_type& widgets;
            Widget *focus_widget = nullptr;//The widget which has focus
            Widget *drag_widget = nullptr;//The Dragging event
            Widget *cur_widget = nullptr;//Mouse point widget
            /**
             * @brief The mouse is pressed
             * 
             * @note This value is used to check the drag status
             */
            bool mouse_pressed = false;
            bool drag_rejected = false;
    };
    class BTKAPI Container:public Widget,private EventDispatcher{
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
            
            
    };
    class Line:public Widget{
        public:
            Line(Window &,Orientation);
            Line(Window &,int x,int y,int w,int h,Orientation);
            ~Line();
            
            void draw(Renderer &);
        private:
            Orientation orientation;
    };
};

#endif // _BTK_WIDGET_HPP_
