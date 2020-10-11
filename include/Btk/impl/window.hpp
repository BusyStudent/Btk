#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <string_view>
#include <functional>
#include <list>
#include "render.hpp"
namespace Btk{
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
        
        void unref();//unref the object
        void ref();//ref the  object

        SDL_Window *win;
        Renderer    render;
        //callbacks
        std::function<bool()> onclose_cb;//CloseWIndow
        std::function<void(std::string_view)> ondropfile_cb;//DropFile
        //widgets
        std::list<Widget*> widgets_list;
        //refcount
        int refcount;
        //BackGroud Color
        SDL_Color bg_color;
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
