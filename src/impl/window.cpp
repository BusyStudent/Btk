#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/window.hpp>
namespace Btk{
    WindowImpl::WindowImpl(const char *title,int x,int y,int w,int h,int flags){
        win = nullptr;
        render = nullptr;
        win = SDL_CreateWindow(title,x,y,w,h,flags);
        if(win == nullptr){
            //Handle err....
        }
        render = new RendererImpl(win);
    }
    WindowImpl::~WindowImpl(){
        delete render;
        SDL_DestroyWindow(win);
    }
}
namespace Btk{
    Window::Window(std::string_view title,int w,int h){
        Init();
        pimpl = new WindowImpl(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,
            h,
            SDL_WINDOW_ALLOW_HIGHDPI
        );
    }
    Window::~Window(){
        delete pimpl;
    }
    bool Window::mainloop(){
        return Main() == 0;
    }
}