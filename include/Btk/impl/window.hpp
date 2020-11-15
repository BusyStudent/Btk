#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <string_view>
#include <functional>
#include <mutex>
#include <list>
#include "render.hpp"
#include "../signal/signal.hpp"
#include "../font.hpp"
namespace Btk{
    class Event;
    //Impl for Window
    struct Renderer;
    struct Widget;
    struct WindowImpl{
        //Init SDL Window
        WindowImpl(const char *title,int x,int y,int w,int h,int flags);
        ~WindowImpl();
        void draw();
        void pixels_size(int *x,int *y);//GetWindowSize
        //tirgger close cb
        
        bool on_close();
        void on_resize(int new_w,int new_h);
        void on_dropfile(std::string_view file);
        //handle event 
        void handle_windowev(const SDL_Event &event);
        void handle_mousemotion(const SDL_Event &event);
        //Dispatch Event to Widgets
        bool dispatch(Event &event);
        
        void unref();//unref the object
        void ref();//ref the  object
        //update wingets postions
        void update_postion();
        SDL_Window *win;
        Renderer    render;
        //Signals
        Signal<void()> sig_leave;//mouse leave
        Signal<void()> sig_enter;//mouse enter
        Signal<bool()> sig_close;//CloseWIndow
        Signal<void(std::string_view)> sig_dropfile;//DropFile
        Signal<void(int new_w,int new_h)> sig_resize;//WindowResize
        //widgets
        std::list<Widget*> widgets_list;
        //refcount
        int refcount;
        //BackGroud Color
        SDL_Color bg_color;
        //Background cursor
        SDL_Cursor *cursor;
        //mutex
        std::recursive_mutex mtx;
        //Rt draw
        Uint32 rt_fps;
        //Widgets Default Font
        Font default_font;
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
