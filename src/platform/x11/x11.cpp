#include "../../build.hpp"

#include <Btk/platform/x11.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/Btk.hpp>
#include <csignal>

#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_system.h>

#include "internal.hpp"
//Need we throw Exception when the error was happened
static bool throw_xlib_err = true;
#ifndef NDEBUG
static void crash_handler(int sig){
    signal(sig,SIG_DFL);
    //reset to default
    signal(sig,SIG_DFL);
    const char *signame;
    if(sig == SIGSEGV){
        signame = "SIGSEGV";
    }
    else if(sig == SIGABRT){
        signame = "SIGABRT";
    }
    else if(sig == SIGILL){
        signame = "SIGILL";
    }
    else{
        signame = "???";
    }
    fprintf(stderr,"Caught signal '%s'\n",signame);
    _Btk_Backtrace();
    //rethrow
    raise(sig);
}
#endif
namespace Btk{
namespace X11{
    //Internal function
    int  XErrorHandler(Display *display,XErrorEvent *event){
        _Btk_Backtrace();
        char buf[128];
        int ret = XGetErrorText(display,event->error_code,buf,sizeof(buf));
        if(ret == -1){
            buf[0] = '\0';
        }
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "[System::X11]At display \033[34m%s\033[0m \033[31m%s\033[0m",
            XDisplayString(display),
            buf);
        return 0;
    }
    XContext GetXContext(SDL_Window *win){
        SDL_SysWMinfo info;
        SDL_GetVersion(&(info.version));
        if(SDL_GetWindowWMInfo(win,&info) == SDL_FALSE){
            throwSDLError();
        }
        //Check is x11
        if(info.subsystem != SDL_SYSWM_X11){
            SDL_Unsupported();
            throwSDLError();
            
        }
        return {
            info.info.x11.display,
            info.info.x11.window
        };
    }
}
}
namespace Btk{
namespace X11{
    
    void Init(){
        #ifndef NDEBUG
        //Debug crash handler
        signal(SIGSEGV,crash_handler);
        signal(SIGABRT,crash_handler);
        signal(SIGILL,crash_handler);
        _Xdebug = 1;
        #endif
        XSetErrorHandler(XErrorHandler);
    }
    void Quit(){

    }
    void HandleSysMsg(const SDL_SysWMmsg &){

    }
}
}

//Some platform depended operations for Window
namespace Btk{
    //< Note This method is not impl yet
    void Window::set_transparent(float value){
        using namespace X11;
        auto [display,window] = GetXContext(pimpl->win);
        
        XVisualInfo vinfo;
        if(not XMatchVisualInfo(display,DefaultScreen(display),32,TrueColor,&vinfo)){
            //Oh no
            throwRuntimeError("XMatchVisualInfo failed");
            return;
        }
        BTK_LOGINFO("XWindow %zd depth:%d",window,vinfo.depth);
        XSetWindowBackground(display,window,0);
    }
}