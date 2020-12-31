#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <string_view>
#include <functional>
#include <mutex>
#include <list>
#include "render.hpp"
#include "../signal/signal.hpp"
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
        void handle_keyboardev(KeyEvent &event);
        void handle_mousemotion(const SDL_Event &event);
        void handle_mousebutton(const SDL_Event &event);
        void handle_textinput(TextInputEvent &event);
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
        Signal<bool(Event &)> sig_event;//Process Unhandled Event
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
        Atomic visible = false;
        //Last draw ticks
        Uint32 last_draw_ticks;
        //Widgets Default Font
        Font default_font;
        //Window theme
        Theme *theme;
        //Mouse Position
        int mouse_x;
        int mouse_y;
        /**
         * @brief The widget which the mouse point in
         * 
         */
        Widget * cur_widget = nullptr;
        /**
         * @brief The widget where the drag begin
         * 
         */
        Widget *drag_widget = nullptr;
        /**
         * @brief The mouse is pressed
         * 
         * @note This var is used to check the drag status
         */
        bool mouse_pressed = false;
        bool drag_rejected = false;
        /**
         * @brief The widget which has focus
         * 
         */
        Widget *focus_widget = nullptr;

        //The draw callback
        //It will be called at last
        std::list<DrawCallback> draw_cbs;
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
