#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/container.hpp>
#include <Btk/exception.hpp>
#include <SDL2/SDL_syswm.h>
namespace Btk{
    #if BTK_STILL_DEV
    //TODO Finish EmbedWindow
    static auto get_win_ptr(SDL_Window *win){
        EmbedWindow::WinPtr ptr;
        //Set native window
        SDL_SysWMinfo info;
        SDL_GetVersion(&(info.version));
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
        if(rectangle().empty()){
            //We should hide the window
            SDL_HideWindow(_window);
        }
        else if(attached_to_parent){
            //Not empty
            //Has attached to the parent
            SDL_ShowWindow(_window);
        }
    }
    void EmbedWindow::draw(Renderer &){
        if(not attached_to_parent){
            //Attached to the window
            auto win = window();
            reparent_window(
                _helper_window,
                WinPtr{win->x_display,win->x_window},
                x(),
                y()
            );
            attached_to_parent = true;
        }
        else{
            //Debug
            int x, y,w,h;
            SDL_GetWindowPosition(_window,&x,&y);
            SDL_GetWindowSize(_window,&w,&h);
            BTK_LOGINFO("%d,%d,%d,%d",x,y,w,h);
            //Ask it to redraw
        }
    }
    void EmbedWindow::set_parent(Widget *w){
        Widget::set_parent(w);
        detach_window();
        attached_to_parent = false;
    }
    void EmbedWindow::set_window(WinPtr win){
        if(has_embed()){
            //Detach ...
        }
        _user_window = win;
        //Attach to the helper
        reparent_window(_user_window,_helper_window,0,0);
    }
    #endif
}