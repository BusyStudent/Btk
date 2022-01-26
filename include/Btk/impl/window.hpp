#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_video.h>
#include <string_view>
#include <atomic>
#include <mutex>
#include <list>
#include "../container.hpp"
#include "../render.hpp"
#include "../string.hpp"
#include "../signal.hpp"
#include "../widget.hpp"
#include "../themes.hpp"
#include "../render.hpp"
#include "../event.hpp"
#include "../font.hpp"
#include "../font.hpp"
#include "atomic.hpp"
namespace Btk{
    class Window;
    class Event;
    //Impl for Window
    class Renderer;
    class Widget;
    class Theme;
    class Menu;
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
    class BTKAPI WindowImpl:public Group{
        //Init SDL Window
        public:
            explicit WindowImpl(SDL_Window *);
            WindowImpl(const WindowImpl &) = delete;
            ~WindowImpl();
            //Overload the method from widget
            void draw(Renderer &render) override;
            /**
             * @brief Send a redraw request
             * 
             */
            void redraw();
            bool handle(Event &) override;
            
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
            void on_dropfile(u8string_view file);
            //handle event 
            void handle_draw(Uint32 ticks);
            void handle_windowev(const SDL_Event &event);
            
            //update wingets postions
            void update_postion();

            Uint32 id() const{
                return SDL_GetWindowID(win);
            }
            SDL_Window *sdl_window() const noexcept{
                return win;
            }
            //Set/Get Userdata
            void  set_userdata(const char *name,void *data){
                SDL_SetWindowData(win,name,data);
            }
            void *userdata(const char *name) const{
                return SDL_GetWindowData(win,name);
            }
            template<class T>
            T    *userdata(const char *name) const{
                return static_cast<T*>(SDL_GetWindowData(win,name));
            }
        public:
            //Process Event
            bool handle_drop(DropEvent     &) override;
            bool handle_mouse(MouseEvent   &) override;
            bool handle_motion(MotionEvent &) override;
        private:
            SDL_Window *win = nullptr;
            //Render's device
            RendererDevice *_device = nullptr;
            Renderer    render;
            //Signals
            Signal<void()> sig_leave;//mouse leave
            Signal<void()> sig_enter;//mouse enter
            Signal<bool()> sig_close;//CloseWIndow
            Signal<void(u8string_view)> sig_dropfile;//DropFile
            Signal<void(int new_w,int new_h)> sig_resize;//WindowResize
            Signal<bool(Event &)> sig_event;//Process Unhandled Event
            //BackGroud Color
            Color bg_color;
            //Background cursor
            SDL_Cursor *cursor = nullptr;
            //mutex
            std::recursive_mutex mtx;
            Atomic visible = false;
            //Last time we call the redraw
            Uint32 last_redraw_ticks = 0;
            //Last time the window was drawed(for drop to last event)
            Uint32 last_draw_ticks = 0;
            //FPS limit(0 on unlimited)
            Uint32 fps_limit = 60;
            //The draw callback
            //It will be called at last
            std::list<DrawCallback> draw_cbs;

            //Current menu bar
            Menu *menu_bar = nullptr;
            
            //Methods for Widget impl
        private:
            /**
             * @brief The mouse is pressed
             * 
             * @note This value is used to check the drag status
             */
            bool mouse_pressed = false;
            bool drag_rejected = false;
            bool dragging = false;
        public:
            #ifdef _WIN32
            //Win32 parts
            Uint32 win32_draw_ticks = 0;
            
            void __stdcall handle_win32(
                void *hwnd,
                unsigned int message,
                Uint64 wParam,
                Sint64 lParam
            );
            //Win32 MessageHook
            Function<void(void *,unsigned int,Uint64,Sint64)> win32_hooks;
            
            #endif

            #ifdef __gnu_linux__
            //X11 parts
            unsigned long x_window;
            void         *x_display;

            /**
             * @brief Method for process XEvent
             * 
             * @param p_xevent const pointer to XEvent
             * @return BTKHIDDEN 
             */
            BTKHIDDEN
            void handle_x11(const void *p_xevent);
            #endif
        friend class Menu;
        friend class Window;
        friend class Widget;
        friend class System;
        friend void PushEvent(Event *,Window &);
    };
}

#endif // _BTKIMPL_WINDOW_HPP_
