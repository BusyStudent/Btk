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
    struct BTKAPI WindowImpl{
        //Init SDL Window
        WindowImpl(const char *title,int x,int y,int w,int h,int flags);
        ~WindowImpl();
        void draw();
        void pixels_size(int *w,int *h);//GetWindowSize
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
        //Window theme
        Theme theme;
        Container container;
        //The draw callback
        //It will be called at last
        std::list<DrawCallback> draw_cbs;

        //Methods for Widget impl
        /**
         * @brief Get window's font
         * 
         * @return Font 
         */
        Font font() const{
            return theme.font;
        }
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
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
