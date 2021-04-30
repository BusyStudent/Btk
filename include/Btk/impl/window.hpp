#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <string_view>
#include <functional>
#include <mutex>
#include <list>
#include "render.hpp"
#include "../signal.hpp"
#include "../widget.hpp"
#include "../themes.hpp"
#include "../event.hpp"
#include "../font.hpp"
#include "atomic.hpp"
namespace Btk{
    class Event;
    //Impl for Window
    struct Renderer;
    struct Widget;
    struct Theme;
    /**
     * @brief The Internal Window Draw Callback
     */
    struct DrawCallback{
        Widget *widget;
        void *userdata;
        bool (*draw_fn)(Renderer &,Widget *widget,void*);
        /**
         * @brief Start Render
         * 
         * @param render The render
         * @return true Keep the function
         * @return false Remove it fron the list
         */
        bool call(Renderer &render) const{
            return draw_fn(render,widget,userdata);
        }
    };
    class BTKAPI WindowImpl:public Widget{
        //Init SDL Window
        public:
            WindowImpl(const char *title,int x,int y,int w,int h,int flags);
            ~WindowImpl();
            //Overload the method from widget
            void draw(Renderer &render);
            bool handle(Event &);
            
            void pixels_size(int *w,int *h);//GetWindowSize
            /**
             * @brief The framebuffer's size
             * 
             * @param w 
             * @param h 
             */
            void buffer_size(int *w,int *h);//GetWindowSize
            //tirgger close cb
            
            bool on_close();
            void on_resize(int new_w,int new_h);
            void on_dropfile(std::string_view file);
            //handle event 
            void handle_windowev(const SDL_Event &event);
            //Dispatch Event to Widgets
            bool dispatch(Event &event);
            
            //update wingets postions
            void update_postion();

            Uint32 id() const{
                return SDL_GetWindowID(win);
            }
        public:
            //Process Event
            bool handle_click(MouseEvent   &);
            bool handle_whell(WheelEvent   &);
            bool handle_motion(MotionEvent &);
            bool handle_keyboard(KeyEvent  &);
            bool handle_textinput(TextInputEvent &);
        private:
            SDL_Window *win = nullptr;
            Renderer    render;
            //Signals
            Signal<void()> sig_leave;//mouse leave
            Signal<void()> sig_enter;//mouse enter
            Signal<bool()> sig_close;//CloseWIndow
            Signal<void(std::string_view)> sig_dropfile;//DropFile
            Signal<void(int new_w,int new_h)> sig_resize;//WindowResize
            Signal<bool(Event &)> sig_event;//Process Unhandled Event
            //BackGroud Color
            Color bg_color;
            //Background cursor
            SDL_Cursor *cursor = nullptr;
            //mutex
            std::recursive_mutex mtx;
            Atomic visible = false;
            //Last draw ticks
            Uint32 last_draw_ticks = 0;
            //FPS limit
            Uint32 fps_limit = 60;
            //Window theme
            Theme theme;
            //The draw callback
            //It will be called at last
            std::list<DrawCallback> draw_cbs;

            //Methods for Widget impl

            /**
             * @brief Add widget in Window
             * 
             * @tparam T 
             * @tparam Args 
             * @param args 
             * @return T& 
             */
            template<class T,class ...Args>
            T &add(Args &&...args){
                T *ptr = new T(container,std::forward<Args>(args)...);
                container.add(ptr);
                return *ptr;
            }
        private:
            //A helper for set the current focus widget
            void set_focus_widget(Widget *);
            void window_mouse_leave();
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
        friend class Window;
        friend class Widget;
        friend class System;
        friend void DispatchEvent(const SDL_Event &ev,void*);
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
