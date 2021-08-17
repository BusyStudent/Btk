#include "../build.hpp"

#include <Btk/container.hpp>
#include <Btk/exception.hpp>
#include <SDL2/SDL_syswm.h>
namespace Btk{
    #if BTK_STILL_DEV
    //TODO Finish EmbedWindow
    static auto get_win_ptr(SDL_Window *win){
        EmbedWindow::WinPtr ptr;
        //Set native window
        SDL_version ver;
        SDL_GetVersion(&ver);
        SDL_SysWMinfo info;
        SDL_GetWindowWMInfo(win,&info);
        //Set value
        #if defined(SDL_VIDEO_DRIVER_X11)
        BTK_ASSERT(info.subsystem == SDL_SYSWM_X11);
        ptr.first = info.info.x11.display;
        ptr.second = info.info.x11.window;
        #endif

        #if defined(SDL_VIDEO_DRIVER_WINDOWS)
        ptr = info.info.win.window;
        #endif
        return ptr;
    }
    EmbedWindow::EmbedWindow(){
        //Create the helper window
        _window = SDL_CreateWindow(
            nullptr,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            1,1,
            SDL_WINDOW_BORDERLESS | 
            SDL_WINDOW_HIDDEN | SDL_WINDOW_ALLOW_HIGHDPI
        );
        if(_window == nullptr){
            throwSDLError();
        }
        _helper_window = get_win_ptr(_window);
    }
    EmbedWindow::~EmbedWindow(){
        detach_window();
        SDL_DestroyWindow(_window);
    }
    void EmbedWindow::set_rect(const Rect &rect){
        Widget::set_rect(rect);
        SDL_SetWindowPosition(_window,rect.x,rect.y);
        SDL_SetWindowSize(_window,rect.w,rect.h);

        nt_set_rect(rect);
    }
    void EmbedWindow::draw(Renderer &){

    }
    void EmbedWindow::set_parent(Widget *w){
        Widget::set_parent(w);
    }
    #endif
}