#if !defined(_BTK_WIDGET_HPP_)
#define _BTK_WIDGET_HPP_
#include <cstdio>
#include <list>
#include "function.hpp"
#include "signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    class Font;
    class Theme;
    class Window;
    class Renderer;
    
    //Event forward decl
    class Event;
    class Widget;
    class Container;

    class WindowImpl;

    struct KeyEvent;
    struct DragEvent;
    struct MouseEvent;
    struct WheelEvent;
    struct MotionEvent;
    struct ResizeEvent;
    struct TextInputEvent;

    enum class FocusPolicy{
        None,
        KeyBoard,
        Click,
        Wheel
    };
    //Alignment
    enum class Align:unsigned int{
        Center,//<V and H
        //Vertical Alignment
        Top,
        Bottom,
        Baseline,//< Only for TextAlign
        //Horizontal Alignment
        Right,
        Left
    };

    //Attribute for Widget
    struct WidgetAttr{
        bool hide = false;//<Is hide
        bool window = false;//<Is window
        bool user_rect = false;//<Using user defined position
        bool container = false;//<Is container
        bool disable = false;//<The widget is disabled?
        FocusPolicy focus = FocusPolicy::None;//<Default the widget couldnot get focus
    };
    /**
     * @brief Helper class for store data
     * 
     */
    struct WidgetHolder{
        WidgetHolder() = default;
        WidgetHolder(const WidgetHolder &) = default;
        WidgetHolder(Widget *w):widget(w){};
        Widget *widget = nullptr;
        void *userdata = nullptr;
        void (*cleanup)(WidgetHolder&) = nullptr;

        void _cleanup(){
            if(cleanup == nullptr){
                return;
            }
            cleanup(*this);
        }
        operator Widget*() const noexcept{
            return widget;
        }
        Widget *operator ->() const noexcept{
            return widget;
        }
        Widget *get() const noexcept{
            return widget;
        }

        template<class T>
        static void DeleteHelper(WidgetHolder &h){
            delete static_cast<T*>(h.userdata);
        }
        template<class T>
        void set_userdata(){
            userdata = new T;
            cleanup = DeleteHelper<T>;
        }
        template<class T>
        void set_userdata(T &&t){
            userdata = new T(std::forward<T>(t));
            cleanup = DeleteHelper<T>;
        }
        template<class T>
        T &get_userdata(){
            return *static_cast<T*>(userdata);
        }
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
    #if 0
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
                    std::forward<Args>(args)...
                );
                add(static_cast<Widget*>(ptr));
                return *ptr;
            }
            bool add(Widget *w);
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
            /**
             * @brief Get the widget's list
             * 
             * @return std::list<Widget*>& 
             */
            std::list<WidgetHolder> &widgets() noexcept{
                return widgets_list;
            }
            const std::list<WidgetHolder> &widgets() const noexcept{
                return widgets_list;
            }
            /**
             * @brief For each widget
             * 
             * @tparam Callable 
             * @tparam Args 
             * @param callable 
             * @param args 
             */
            template<class Callable,class ...Args>
            void for_each(Callable &&callable,Args &&...args){
                for(auto w:widgets_list){
                    callable(w,std::forward<Args>(args)...);
                }
            }
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
            /**
             * @brief The window's mouse was left(Internal)
             * 
             * @internal User should not use it
             */
            BTKHIDDEN void window_mouse_leave();
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
            std::list<WidgetHolder> widgets_list;
        friend class  Window;
        friend class  Widget;
        friend struct System;
        friend class  WindowImpl;
    };
    #endif
    class BTKAPI Widget:public HasSlots{
        public:
            /**
             * @brief Construct a new Widget object
             * @note All data will be inited to 0
             */
            Widget();
            Widget(Widget *parent);
            Widget(Widget &parent):Widget(&parent){}

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
            virtual void set_rect(const Rect &rect);

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
            void set_rect(int x,int y,int w,int h){
                 set_rect({x,y,w,h});
            }
            void set_position(const Vec2 &vec2){
                set_rect(vec2.x,vec2.y,rect.w,rect.h);
            }
            int x() const noexcept{
                return rect.x;
            }
            int y() const noexcept{
                return rect.y;
            }
            int w() const noexcept{
                return rect.w;
            }
            int h() const noexcept{
                return rect.h;
            }
            bool is_enable() const noexcept{
                return not attr.disable;
            }
            /**
             * @brief A function for getting rect

             * @return Rect 
             */
            Rect rectangle() const noexcept{
                return rect;
            }
            /**
             * @brief A template for get FRect like this rectangle<float>()
             * 
             * @tparam T 
             * @tparam RetT 
             * @return RetT 
             */
            template<class T,class RetT = FRect>
            RetT rectangle() const noexcept;
            /**
             * @brief Show the widget tree 
             * 
             */
            void dump_tree(FILE *output = stderr);
            /**
             * @brief Set the parent (you can overload it)
             * 
             * @param parent The pointer to the parent
             */
            virtual void set_parent(Widget *parent);
            Widget *parent() const noexcept{
               return _parent; 
            }
            /**
             * @brief Delete all childrens
             * 
             */
            void clear_childrens();
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
            WindowImpl *window() const noexcept;
            /**
             * @brief Get current window's renderer
             * 
             * @return Renderer* (failed on nullptr)
             */
            Renderer *renderer() const;
            /**
             * @brief Get the default font
             * 
             * @return Font 
             */
            Font default_font() const;
            /**
             * @brief Get the theme of the current window
             * 
             * @return Theme& 
             */
            Theme &window_theme() const;

            
        public:
            //Event Handle Method,It will be called in Widget::handle()
            /**
             * @brief Called in Drag(type Is Drag DragBegin DragEnd)
             * 
             * @return true 
             * @return false 
             */
            virtual bool handle_drag(DragEvent     &){return false;}
            virtual bool handle_click(MouseEvent   &){return false;}
            virtual bool handle_whell(WheelEvent   &){return false;}
            virtual bool handle_motion(MotionEvent &){return false;}
            virtual bool handle_keyboard(KeyEvent  &){return false;}
            virtual bool handle_textinput(TextInputEvent &){return false;}
            
        protected:
            WidgetAttr attr;//Widget attributes
            Rect rect = {0,0,0,0};//Widget rect

            std::list<Widget*> childrens;
        private:
            Widget *_parent = nullptr;//< Parent
            mutable WindowImpl *_window = nullptr;//<Window pointer
        friend class Window;
        friend class Layout;
        friend class WindowImpl;
        friend class Container;
        friend void  PushEvent(Event *,Widget &);
    };

    class BTKAPI Line:public Widget{
        public:
            Line(Orientation);
            Line(int x,int y,int w,int h,Orientation);
            ~Line();
            
            void draw(Renderer &);
        private:
            Orientation orientation;
    };
    template<>
    inline FRect Widget::rectangle<float,FRect>() const noexcept{
        return FRect(rect);
    }
}

#endif // _BTK_WIDGET_HPP_
