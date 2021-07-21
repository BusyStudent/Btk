#include "../../build.hpp"

#include <Btk/platform/x11.hpp>
#include <Btk/platform/fs.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/widget.hpp>
#include <Btk/Btk.hpp>
#include <csignal>

#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_system.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

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
    bool has_zenity = false;
    bool has_kdialog = false;
    //Map X11's window to btk's wiondow
    static ObjectHolder<std::map<XWindow,WindowImpl*>> wins_map;

    static void on_add_window(WindowImpl *win){
        //set window 
        auto [display,window] = GetXContext(win->sdl_window());

        win->x_window = window;
        win->x_display = display;
        //Add to map
        wins_map->emplace(std::make_pair(window,win));
    }
    static void on_del_window(WindowImpl *win){
        //Remove it
        wins_map->erase(BTK_X_WINDOW(win->x_window));
    }

    void Init(){
        #ifndef NDEBUG
        //Debug crash handler
        signal(SIGSEGV,crash_handler);
        signal(SIGABRT,crash_handler);
        signal(SIGILL,crash_handler);
        _Xdebug = 1;
        #endif
        XSetErrorHandler(XErrorHandler);
        u8string buf;
        //Find zenity and kdialog
        ForPath([&](u8string_view fdir){
            buf = fdir;
            //add it
            if(buf.back() != '/'){
                buf.push_back('/');
            }
            if(not has_zenity){
                has_zenity = exists(buf + "zenity");
            }
            if(not has_kdialog){
                has_kdialog = exists(buf + "kdialog");
            }
            if(has_kdialog and has_zenity){
                BTK_LOGINFO("Zenity and kdialog found");
                return false;
            }
            return true;
        });
        //Hook window create
        wins_map.construct();

        Instance().signal_window_created.connect(on_add_window);

        SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
    }
    void Quit(){
        wins_map.destroy();
    }
    void HandleSysMsg(const SDL_SysWMmsg &msg){
        const XEvent &event = msg.msg.x11.event;
        switch(event.type){

        }
    }
    //Exec
    int VExecute(size_t argc,...){
        va_list varg;
        va_start(varg,argc);

        //Allocate args array
        char **args = static_cast<char**>(alloca((argc + 1) * sizeof(char*)));
        size_t i;
        for(i = 0;i < argc;i++){
            args[i] = va_arg(varg,char*);
        }
        args[argc] = nullptr;
        va_end(varg);


        pid_t pid = fork();
        if(pid == -1){
            return -1;
        }
        else if(pid == 0){
            execvp(args[0],args);
            _Exit(-1);
        }
        else{
            int ret;
            waitpid(pid,&ret,0);
            return ret;
        }
    }
    FILE *VPopen(size_t argc,...){
        va_list varg;
        va_start(varg,argc);

        //Allocate args array
        char **args = static_cast<char**>(alloca((argc + 1) * sizeof(char*)));
        size_t i;
        for(i = 0;i < argc;i++){
            args[i] = va_arg(varg,char*);
        }
        args[argc] = nullptr;
        va_end(varg);

        int fds[2];
        int err_fds[2];
        pipe(fds);
        pipe2(err_fds,O_CLOEXEC);

        pid_t pid = fork();
        if(pid == -1){
            close(fds[0]);
            close(fds[1]);
            return nullptr;
        }
        else if(pid == 0){
            close(fds[0]);
            close(err_fds[0]);
            dup2(fds[1],STDOUT_FILENO);
            //Exec it
            execvp(args[0],args);
            //Failed wrtie errno
            auto err = errno;
            write(err_fds[1],&err,sizeof(err));
            _Exit(-1);
        }
        else{
            auto handler = std::signal(SIGPIPE,SIG_IGN);
            close(fds[1]);
            close(err_fds[1]);
            int error;
            if(read(err_fds[0],&error,sizeof(error)) == sizeof(error)){
                //has error
                //cleanup
                std::signal(SIGPIPE,handler);
                close(fds[0]);
                close(err_fds[0]);

                return nullptr;
            }
            close(err_fds[0]);
            std::signal(SIGPIPE,handler);
            //Convert it to STD FILE*
            FILE *fptr = fdopen(fds[0],"r");
            if(fptr == nullptr){
                //Error,close the fd
                close(fds[0]);
            }
            return fptr;
        }
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
        XSetWindowAttributes attr;
        attr.background_pixel = 0;
        attr.border_pixel = 0;
        XChangeWindowAttributes(display,window,CWBorderPixel | CWBackPixel,&attr);
        pimpl->bg_color.a = 0;
    }
    bool HideConsole(){
        return daemon(1,0) == 0;
    }

    void EmbedWindow::set_window(void *u_display,unsigned long u_win){
        //TODO 
        X11::XWindow xwin = BTK_X_WINDOW(u_win);
        //Get display and window
        // XReparentWindow(display,p_win,xwin,0,0);
        XFlush(BTK_X_DISPLAY(_display));

        
    }
}