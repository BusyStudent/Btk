#if !defined(_BTKIMPL_WINDOW_HPP_)
#define _BTKIMPL_WINDOW_HPP_
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <list>
namespace Btk{
    //Impl for Window
    struct RendererImpl;
    struct WindowImpl{
        //Init SDL Window
        WindowImpl(const char *title,int x,int y,int w,int h,int flags);
        ~WindowImpl();
        SDL_Window *win;
        RendererImpl *render;
    };
};

#endif // _BTKIMPL_WINDOW_HPP_
