#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <list>
#include "function.hpp"
#include "signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    class Font;
    class Window;
    class Renderer;
    //Event forward decl
    class Event;
    class Widget;
    class Container;

    struct WindowImpl;

    struct KeyEvent;
    struct MouseEvent;
    struct WheelEvent;
    struct MotionEvent;
    struct TextInputEvent;

    enum class FocusPolicy{
        None,
        KeyBoard,
        Click,
        Wheel
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
    #if 0
    /**
     * @brief A helper of dispatch event in widget
     * 
     */
    class BTKAPI EventDispatcher{
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
            /**
             * @brief Manager by window
             * 
             */
            bool managed_window = false;
        friend struct WindowImpl;
    };
    #endif
    /**
     * @brief A Container of Widget
     * 
     */
    class BTKAPI Container{
        public:
            Container();
            Container(const Container &) = delete;
            ~Container();
            /**
             * @brief The event filter(return false to drop the event)
             * 
             */
            typedef Function<bool(Event&)> Filter;
        public:
            /**
             * @brief Add a widget to the Container
             * 
             * @tparam T 
             * @tparam Args 
             * @param args 
             * @return The new widget ref
             */
            template<class T,class ...Args>
            T& add(Args &&...args){
                T *ptr = new T(
                    *this,
                    std::forward<Args>(args)...
                );
                widgets_list.push_back(ptr);
                return *ptr;
            }
            bool add(Widget *w){
                if(w != nullptr){
                    widgets_list.push_back(w);
                    return true;
                }
                return false;
            }
            /**
             * @brief Destroy all widget
             * 
             */
            void clear();
            /**
             * @brief Remove the widget in the container
             * 
             * @param widget The widget pointer
             * @return true Successed to remove it
             * @return false The widget pointer is invaid
             */
            bool remove(Widget *widget);
            /**
             * @brief Detach the widget in the container
             * 
             * @param widget The widget pointer
             * @return true Successed to Detach it
             * @return false The widget pointer is invaid
             */
            bool detach(Widget *widget);
        public:
            //Process Event
            bool handle_click(MouseEvent   &);
            bool handle_whell(WheelEvent   &);
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
            /**
             * @brief Get the Container EventFilter
             * 
             * @return Function<bool(Event&)>& 
             */
            Function<bool(Event&)> &filter() noexcept{
                return ev_filter;
            }
        protected:
            //A helper for set the current focus widget
            void set_focus_widget(Widget *);
            
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
            /**
             * @brief Manager by window
             * 
             */
            bool managed_window = false;
            /**
             * @brief Event filter
             * 
             */
            Function<bool(Event&)> ev_filter;
        protected:
            //top window
            WindowImpl *window;
            std::list<Widget*> widgets_list;
        friend class  Window;
        friend class  Widget;
        friend struct System;
        friend struct WindowImpl;
    };
    class BTKAPI Widget:public HasSlots{
        public:
            /**
             * @brief Construct a new Widget object
             * @note All data will be inited to 0
             */
            Widget():attr(),rect(
                0,0,0,0
            ),parent(nullptr){};
            Widget(Container *parent):Widget(){
                this->parent = parent;
            }
            Widget(Container &parent):Widget(){
                this->parent = &parent;
            }

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
            /**
             * @brief Return The widget's master
             * 
             * @return Window ref
             */
            Window &master() const;
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
            /**
             * @brief Get the container of the widget
             * 
             * @return Container& 
             */
            Container& container() const;
            /**
             * @brief Get the top container
             * 
             * @return Container& 
             */
            Container& top_container() const;

        protected:
            /**
             * @brief Send a redraw request to the window
             * 
             */
            void redraw();
            /**
             * @brief Get current window
             * 
             * @return WindowImpl* 
             */
            WindowImpl *window() const noexcept{
                return parent->window;
            }
            /**
             * @brief Get the default font
             * 
             * @return Font 
             */
            Font default_font() const;
        protected:
            WidgetAttr attr;//Widget attributes
            Rect rect;//Widget rect
            Container *parent;//Parents
        friend class  Window;
        friend class  Layout;
        friend struct WindowImpl;
        friend class  Container;
        friend void   PushEvent(Event *,Widget &);
    };

    class BTKAPI Line:public Widget{
        public:
            Line(Container &,Orientation);
            Line(Container &,int x,int y,int w,int h,Orientation);
            ~Line();
            
            void draw(Renderer &);
        private:
            Orientation orientation;
    };
};

#endif // _BTK_WIDGET_HPP_
