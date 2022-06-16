#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include "container.hpp"
#include "string.hpp"
#include "rect.hpp"
#include "defs.hpp"
#include <cstdio>
#include <atomic>
#include <mutex>

//Debug macro
#define BTK_DEBUGHINT_WINDOW_DRAW_BOUNDS() ::setenv("BTK_DRAW_BOUNDS", "1", 1);

namespace Btk{
    class RendererDevice;
    class Container;
    class GLDevice;
    class MenuBar;
    class PixBuf;
    class Widget;
    class Event;
    class Font;
    enum class WindowFlags:Uint32{
        None        = 0, //< Default
        OpenGL      = 1 << 0, //< Use OpenGL
        Vulkan      = 1 << 1, //< Use Vulkan
        SkipTaskBar = 1 << 2, //< Skip taskbar
        OpenGLMSAA  = 1 << 3, //< Use OpenGL Multisample
        Transparent = 1 << 4, //< Background transparent
        Resizeable  = 1 << 5, //< Resizeable
        Borderless  = 1 << 6, //< Borderless
        PopupMenu   = 1 << 7, //< Treat as a popup menu
        Fullscreen  = 1 << 8, //< Fullscreen
    };
    BTK_FLAGS_OPERATOR(WindowFlags,Uint32);
    /**
     * @brief Class for describe native window
     * 
     */
    class NativeWindow;
    /**
     * @brief A Basic Window 
     * 
     */
    class BTKAPI Window:public Group{
        public:
            //Signals
            typedef Signal<bool()> SignalClose;
            typedef Signal<bool(Event&)> SignalEvent;
            typedef Signal<void(int x,int y)> SignalMove;
            typedef Signal<void(int w,int h)> SignalResize;
            typedef Signal<void(u8string_view)> SignalDropFile;
            //Flags
            using Flags = WindowFlags;
        public:
            Window(const Window &) = delete;
            /**
             * @brief Construct a new Window object
             * 
             * @param title The window title
             * @param w The window width
             * @param h Thw window height
             */
            //Window(std::string_view title,int w,int h);
            explicit Window(u8string_view title,int w,int h,Flags f = Flags::None);
            /**
             * @brief Construct a new Window object by native window handle
             * 
             * @param native_handle (reinterpret_cast<const NativeWindow*>(your_handle))
             */
            explicit Window(const NativeWindow *native_handle);
            /**
             * @brief Destroy the Window object
             * 
             */
            ~Window();

            inline Uint32 id() const noexcept{
                return winid;
            }
            /**
             * @brief Draw the window
             * 
             */
            void draw(Renderer &,Uint32) override;
            /**
             * @brief Handle the event
             * 
             * @param event 
             * @return true 
             * @return false 
             */
            bool handle(Event  &event) override final;
            /**
             * @brief Update widgets position
             * 
             */
            void update();
            /**
             * @brief Lock the window before access it
             * 
             */
            void lock();
            /**
             * @brief Unlock thw window
             * 
             */
            void unlock();
            /**
             * @brief Show window and set Widget postions
             * 
             */
            void done();
            /**
             * @brief Move the window
             * 
             * @param x The new x
             * @param y The new y
             */
            void move  (int x,int y);
            void resize(int w,int h);
            /**
             * @brief Make the window visiable
             * 
             */
            void show();
            /**
             * @brief Raise the window to top
             * 
             */
            void raise();
            /**
             * @brief Redraw the window
             * 
             * @note Is is safe to call in multithreading without lock it
             */
            void redraw();

            /**
             * @brief Send a close request
             * 
             * @note Is is safe to call in multithreading without lock it
             * 
             * @return true on window is closed
             * @return false on the request is pending or canceled
             */
            bool close();
            /**
             * @brief Enter the main event loop
             * 
             * @return true The event loop finished normally
             * @return false Double call
             */
            bool mainloop();
            /**
             * @brief Get window's pixbuf
             * 
             * @return PixBuf 
             */
            PixBuf pixbuf();
            /**
             * @brief Set the title 
             * 
             * @param title The new title
             */
            void set_title(u8string_view title);
            /**
             * @brief Set the icon 
             * 
             * @param file The image file name
             */
            void set_icon(u8string_view file);
            /**
             * @brief Set the icon 
             * 
             * @param pixbuf The image pixbuf
             */
            void set_icon(const PixBuf &pixbuf);
            /**
             * @brief Set the fullscreen
             * 
             * @param val The fullscreen flags
             */
            void set_fullscreen(bool val = true);
            /**
             * @brief Set the resizeable 
             * 
             * @param val The resizeable flags
             */
            void set_resizeable(bool val = true);
            /**
             * @brief Set the boardered 
             * 
             * @param val The boarder
             */
            void set_boardered(bool val = true);
            /**
             * @brief Set the rt fps object
             * 
             * @param fps The fps object(0 means disable)
             */
            void set_rt_fps(Uint32 fps);
            /**
             * @brief Set the modal object
             * 
             * @param val 
             */
            void set_modal(bool val = true);

            //Process Event
            bool handle_sdl(Event          &)         ;
            bool handle_drop(DropEvent     &) override;
            bool handle_mouse(MouseEvent   &) override;
            bool handle_motion(MotionEvent &) override;
            void handle_draw(Uint32 time    )         ;
            //Get information
            int w() const noexcept;//get w
            int h() const noexcept;//get h
            u8string_view title() const;
            Point      position() const;
            Size           size() const;
            Point mouse_position() const;

            void query_dpi(float *ddpi,float *hdpi,float *vdpi);

            [[nodiscard]]
            SDL_Window *sdl_window() const noexcept{
                return win;
            }

            //Expose signals
            [[nodiscard]]
            auto signal_leave() noexcept -> Signal<void()> &{
                return _signal_leave;
            }
            [[nodiscard]]
            auto signal_enter() noexcept -> Signal<void()> &{
                return _signal_enter;
            }
            [[nodiscard]]
            auto signal_closed() noexcept -> Signal<void()> &{
                return _signal_closed;
            }
            [[nodiscard]]
            auto signal_close() noexcept -> Signal<void(bool &)> &{
                return _signal_close;
            }
            [[nodiscard]]
            auto signal_resize() noexcept -> Signal<void(int,int)> &{
                return _signal_resize;
            }
            [[nodiscard]]
            auto signal_dropfile() noexcept -> Signal<void(u8string_view)> &{
                return _signal_dropfile;
            }
            [[nodiscard]]
            auto signal_event() noexcept -> Signal<bool(Event&)> &{
                return _signal_event;
            }
            [[nodiscard]]
            auto signal_keyboard_take_focus() noexcept -> Signal<void()> &{
                return _signal_keyboard_take_focus;
            }
            [[nodiscard]]
            auto signal_keyboard_lost_focus() noexcept -> Signal<void()> &{
                return _signal_keyboard_lost_focus;
            }

            [[nodiscard]]
            auto flags() const noexcept -> WindowFlags{
                return win_flags;
            }
        private:
            //Methods for Widget impl
            Uint32 rt_timer_cb(Uint32 interval);
            //Initilize the window
            void   initlialize(SDL_Window *win);
            void   set_rect (const Rect &r) override;
        private:
            SDL_Window *win = nullptr;
            //The renderer
            Renderer       *_render = nullptr;
            //Render's device
            RendererDevice *_device = nullptr;
            //Signals
            Signal<void()> _signal_leave;//mouse leave
            Signal<void()> _signal_enter;//mouse enter
            Signal<void()> _signal_closed;//Is closed
            Signal<void(bool &cancel)>  _signal_close;//Will be close
            Signal<void(u8string_view)> _signal_dropfile;//DropFile
            Signal<void(int new_w,int new_h)> _signal_resize;//WindowResize
            Signal<void(int new_x,int new_y)> _signal_moved;//WindowMove
            Signal<bool(Event &)> _signal_event;//Process Unhandled Event
            Signal<void()> _signal_keyboard_take_focus;
            Signal<void()> _signal_keyboard_lost_focus;
            //BackGroud Color
            Color bg_color;
            //Background cursor
            SDL_Cursor *cursor = nullptr;
            //mutex
            std::recursive_mutex mtx;
            //Last time we call the redraw
            Uint32 last_redraw_ticks = 0;
            //Last time the window was drawed(for drop to last event)
            Uint32 last_draw_ticks = 0;
            //FPS limit(0 on unlimited)
            Uint32 fps_limit = 60;
            //Window flags
            Uint32 last_win_flags = 0;//< The last window flags
            //Window ID
            Uint32 winid;
            //Draw event event pending in the queue
            Uint32 draw_event_counter = 0;
            //Rt draw timer / variable
            int    rt_draw_timer = 0;
            Uint32 rt_draw_fps = 0;
            //Window Create Flags
            WindowFlags win_flags = {};
            //Internal state
            /**
             * @brief The mouse is pressed
             * 
             * @note This value is used to check the drag status
             */
            bool mouse_pressed = false;
            bool drag_rejected = false;
            bool dragging = false;
            bool modal = false;//< Is the window modal?
            bool registered = false;//< Is registered to System to recevice events?

            //Current menu bar
            MenuBar *menu_bar = nullptr;

            bool debug_draw_bounds = false;
            bool flat_widget = true;//< Flat the widget if only one child
        public:
            //Platform specific
            #ifdef _WIN32
            //Win32 parts
            Uint32 win32_draw_ticks = 0;
            // Process draw event in win event loop when sizing
            bool win32_sizing_draw = true;
            
            void BTKFAST handle_win32(
                void *hwnd,
                unsigned int message,
                Uint64 wParam,
                Sint64 lParam
            );
            void BTKFAST win32_poll_draw();
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
        friend class System;
        friend class Widget;
    };
    /**
     * @brief OpenGL Window
     * 
     */
    class GLWindow:public Window{
        public:
            GLWindow() = default;
            GLWindow(GLWindow &&) = default;
            GLWindow(u8string_view title,int w,int h,Flags f = Flags::None):
                Window(title,w,h,f + WindowFlags::OpenGL){

            }

            GLDevice *gl_device(){
                return static_cast<GLDevice*>(userdata("btk_dev"));
            }
    };
    /**
     * @brief Get the Screen Size object
     * @param display The display index(0 on default)
     * @return BTKAPI 
     */
    BTKAPI Size    GetScreenSize(int display = 0);
    BTKAPI Window *GetKeyboardFocus();
    /**
     * @brief Get the Mouse Focus Window
     * 
     * @return BTKAPI* 
     */
    BTKAPI Window *GetMouseFocus();
}
#endif // _BTK_WINDOW_HPP_
