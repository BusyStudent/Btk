#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_

#include "../window.hpp"
#include "../render.hpp"
#include "../event.hpp"

//Hint for Window
#define BTK_WINDOWHINT_GLLOADER "btk_gl_loader"
#define BTK_WINDOWHINT_VKLOADER "btk_vk_loader"

namespace Btk{
    // class Window;
    // class Event;
    // //Impl for Window
    // class Renderer;
    // class MenuBar;
    // class Widget;
    // class Theme;
    // //Enum
    // enum class WindowFlags:Uint32;
    // /**
    //  * @brief The Internal Window Draw Callback
    //  */
    // struct DrawCallback{
    //     Widget *widget;
    //     void *userdata;
    //     bool (*draw_fn)(Renderer &,Widget *widget,void*);
    //     /**
    //      * @brief Start Render
    //      * 
    //      * @param render The render
    //      * @return true Keep the function
    //      * @return false Remove it fron the list
    //      */
    //     bool call(Renderer &render) const{
    //         return draw_fn(render,widget,userdata);
    //     }
    // };
    /**
     * @brief WindowImpl
     * 
     */
    // class BTKAPI WindowImpl:public Group{
    //     //Init SDL Window
    //     public:
    //         explicit WindowImpl(SDL_Window *);
    //         WindowImpl(const WindowImpl &) = delete;
    //         ~WindowImpl();
    //         //Overload the method from widget
    //         void draw(Renderer &render,Uint32 timestamp) override;
    //         /**
    //          * @brief Send a redraw request
    //          * 
    //          */
    //         void redraw();
    //         /**
    //          * @brief Send a close request
    //          * 
    //          */
    //         void close();
    //         void move(int x,int y);
    //         void raise();

    //         bool handle(Event &) override;

    //         Point position() const;
    //         Size  size() const;
            
    //         [[deprecated("Use size() instead")]]
    //         void pixels_size(int *w,int *h);//GetWindowSize
    //         /**
    //          * @brief The framebuffer's size
    //          * 
    //          * @param w 
    //          * @param h 
    //          */
    //         [[deprecated]]
    //         void buffer_size(int *w,int *h);//GetWindowSize
    //         //tirgger close cb
            
    //         bool on_close();
    //         void on_resize(int new_w,int new_h);
    //         void on_dropfile(u8string_view file);
    //         //handle event 
    //         void handle_draw(Uint32 ticks);
    //         void handle_windowev(const SDL_Event &event);
            
    //         //update wingets postions
    //         void update_postion();

    //         Uint32 id() const{
    //             return winid;
    //         }
    //         SDL_Window *sdl_window() const noexcept{
    //             return win;
    //         }
    //         void set_modal_for(WindowImpl *parent){
    //             if(parent != nullptr){
    //                 SDL_SetWindowModalFor(win,parent->win);
    //                 set_modal(true);
    //             }
    //             else{
    //                 SDL_SetWindowModalFor(win,nullptr);
    //                 set_modal(false);
    //             }
    //         }
    //         /**
    //          * @brief Get the mouse position in local coordinates
    //          * 
    //          * @return Point 
    //          */
    //         [[nodiscard]]
    //         Point mouse_position() const{
    //             Point p;
    //             Point loc;
    //             SDL_GetGlobalMouseState(&p.x,&p.y);
    //             SDL_GetWindowPosition(win,&loc.x,&loc.y);
    //             return {
    //                 p.x - loc.x,
    //                 p.y - loc.y
    //             };
    //         }
    //         /**
    //          * @brief Set the modal flags
    //          * 
    //          * @param v 
    //          */
    //         void set_modal(bool v = true);
    //         /**
    //          * @brief Set the input focus 
    //          * 
    //          * @param v 
    //          */
    //         void set_input_focus();
    //         /**
    //          * @brief Set the Real time refresh fps(0 on disable)
    //          * 
    //          * @note Set it only your want to refresh your app in a fps
    //          * @param fps 
    //          */
    //         void set_rt_fps(Uint32 fps);
    //         void query_dpi(float *ddpi,float *hdpi,float *vdpi);
    //         /**
    //          * @brief Get the Real time refresh fps
    //          * 
    //          * @return Uint32 
    //          */
    //         Uint32 rt_fps() const noexcept{
    //             return rt_draw_fps;
    //         }

    //         //Expose signals
    //         [[nodiscard]]
    //         auto signal_leave() noexcept -> Signal<void()> &{
    //             return _signal_leave;
    //         }
    //         [[nodiscard]]
    //         auto signal_enter() noexcept -> Signal<void()> &{
    //             return _signal_enter;
    //         }
    //         [[nodiscard]]
    //         auto signal_close() noexcept -> Signal<bool()> &{
    //             return _signal_close;
    //         }
    //         [[nodiscard]]
    //         auto signal_closed() noexcept -> Signal<void()> &{
    //             return _signal_closed;
    //         }
    //         [[nodiscard]]
    //         auto signal_resize() noexcept -> Signal<void(int,int)> &{
    //             return _signal_resize;
    //         }
    //         [[nodiscard]]
    //         auto signal_dropfile() noexcept -> Signal<void(u8string_view)> &{
    //             return _signal_dropfile;
    //         }
    //         [[nodiscard]]
    //         auto signal_event() noexcept -> Signal<bool(Event&)> &{
    //             return _signal_event;
    //         }
    //         [[nodiscard]]
    //         auto signal_keyboard_take_focus() noexcept -> Signal<void()> &{
    //             return _signal_keyboard_take_focus;
    //         }
    //         [[nodiscard]]
    //         auto signal_keyboard_lost_focus() noexcept -> Signal<void()> &{
    //             return _signal_keyboard_lost_focus;
    //         }

    //         [[nodiscard]]
    //         auto flags() const noexcept -> WindowFlags{
    //             return win_flags;
    //         }
    //     public:
    //         //Process Event
    //         bool handle_drop(DropEvent     &) override;
    //         bool handle_mouse(MouseEvent   &) override;
    //         bool handle_motion(MotionEvent &) override;
    //     private:
    //         SDL_Window *win = nullptr;
    //         //The renderer
    //         Constructable<Renderer> render;
    //         //Render's device
    //         RendererDevice *_device = nullptr;
    //         //Signals
    //         Signal<void()> _signal_leave;//mouse leave
    //         Signal<void()> _signal_enter;//mouse enter
    //         Signal<bool()> _signal_close;//Will be close
    //         Signal<void()> _signal_closed;//Is closed
    //         Signal<void(u8string_view)> _signal_dropfile;//DropFile
    //         Signal<void(int new_w,int new_h)> _signal_resize;//WindowResize
    //         Signal<void(int new_x,int new_y)> _signal_moved;//WindowMove
    //         Signal<bool(Event &)> _signal_event;//Process Unhandled Event
    //         Signal<void()> _signal_keyboard_take_focus;
    //         Signal<void()> _signal_keyboard_lost_focus;
    //         //BackGroud Color
    //         Color bg_color;
    //         //Background cursor
    //         SDL_Cursor *cursor = nullptr;
    //         //mutex
    //         std::recursive_mutex mtx;
    //         Atomic visible = false;
    //         //Last time we call the redraw
    //         Uint32 last_redraw_ticks = 0;
    //         //Last time the window was drawed(for drop to last event)
    //         Uint32 last_draw_ticks = 0;
    //         //FPS limit(0 on unlimited)
    //         Uint32 fps_limit = 60;
    //         //Window flags
    //         Uint32 last_win_flags = 0;//< The last window flags
    //         //Window ID
    //         Uint32 winid;
    //         //Draw event event pending in the queue
    //         Uint32 draw_event_counter = 0;
    //         //Rt draw timer / variable
    //         int    rt_draw_timer = 0;
    //         Uint32 rt_draw_fps = 0;
    //         //Window Create Flags
    //         WindowFlags win_flags = {};

    //         //Current menu bar
    //         MenuBar *menu_bar = nullptr;

    //         bool debug_draw_bounds = false;
    //         bool flat_widget = true;//< Flat the widget if only one child
            
    //         //Methods for Widget impl
    //         Uint32 rt_timer_cb(Uint32 interval);
    //     private:
    //         /**
    //          * @brief The mouse is pressed
    //          * 
    //          * @note This value is used to check the drag status
    //          */
    //         bool mouse_pressed = false;
    //         bool drag_rejected = false;
    //         bool dragging = false;
    //         //Modal Datas
    //         bool modal = false;//< Is the window modal?
    //     public:
    //         #ifdef _WIN32
    //         //Win32 parts
    //         Uint32 win32_draw_ticks = 0;
    //         // Process draw event in win event loop when sizing
    //         bool win32_sizing_draw = true;
            
    //         void BTKFAST handle_win32(
    //             void *hwnd,
    //             unsigned int message,
    //             Uint64 wParam,
    //             Sint64 lParam
    //         );
    //         void BTKFAST win32_poll_draw();
    //         //Win32 MessageHook
    //         Function<void(void *,unsigned int,Uint64,Sint64)> win32_hooks;
            
    //         #endif

    //         #ifdef __gnu_linux__
    //         //X11 parts
    //         unsigned long x_window;
    //         void         *x_display;

    //         /**
    //          * @brief Method for process XEvent
    //          * 
    //          * @param p_xevent const pointer to XEvent
    //          * @return BTKHIDDEN 
    //          */
    //         BTKHIDDEN
    //         void handle_x11(const void *p_xevent);
    //         #endif
    //     friend class Menu;
    //     friend class Window;
    //     friend class Widget;
    //     friend class System;
    //     friend void PushEvent(Event *,Window &);
    // };
    /**
     * @brief Create a Window object
     * 
     * @param title 
     * @param w 
     * @param h 
     * @param flags
     * @return The WindowImpl handler
     */
    BTKAPI SDL_Window *CreateWindow(u8string_view title,int w,int h,WindowFlags f);
}

#endif // _BTKIMPL_WINDOW_HPP_
