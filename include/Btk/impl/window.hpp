#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <string_view>
#include <functional>
#include <list>
namespace Btk{
    //Impl for Window
    struct RendererImpl;
    struct WindowImpl{
        //Init SDL Window
        WindowImpl(const char *title,int x,int y,int w,int h,int flags);
        ~WindowImpl();
        void draw();
        //tirgger close cb
        bool close();

        void dropfile(std::string_view file);
        
        void unref();//unref the object
        void ref();//ref the  object

        SDL_Window *win;
        RendererImpl *render;
        //callbacks
        std::function<bool()> onclose_cb;//CloseWIndow
        std::function<void(std::string_view)> ondropfile_cb;//DropFile
        //refcount
        int refcount;
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
