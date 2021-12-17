#include "../../build.hpp"

#include <Btk/platform/popen.hpp>
#include <Btk/platform/x11.hpp>
#include <Btk/platform/fs.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/widget.hpp>
#include <Btk/Btk.hpp>
#include <csignal>
#include <map>

#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_hints.h>
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

#if 1
Btk_CallOnLoad{
    XInitThreads();    
};
#endif

namespace Btk{
namespace X11{
    //Internal function
    int  XErrorHandler(Display *display,XErrorEvent *event){
        
        #ifndef NDEBUG
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
        #endif
        if(throw_xlib_err){
            //Throw XError in MainLoop
            DeferCall(std::rethrow_exception,std::make_exception_ptr<XError>(event));
        }
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
    static Constructable<std::map<XWindow,WindowImpl*>> wins_map;

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
        //Set SDL Hit
        //Disable the compositor
        //Beacuse it will cause a render error in KDE
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR,"0");

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
        //Map it
        XWindow win = event.xany.window;
        auto iter = wins_map->find(win);
        if(iter != wins_map->end()){
            // iter->second->handle_x11(&event);
        }
    }
    //Exec
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
            //Failed, wrtie errno
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

namespace Btk{
    //spawn
    process_t _vspawn(size_t argc,const _vspawn_arg arr[]){
        //Allocate args array
        char **args = static_cast<char**>(alloca((argc + 1) * sizeof(char*)));
        size_t i;
        for(i = 0;i < argc;i++){
            auto &data = arr[i];
            args[i] = static_cast<char*>(alloca(data.n + 1));
            memcpy(args[i],data.str,data.n);
            args[i][data.n] = '\0';
        }
        args[argc] = nullptr;
        //alloc error report pipe
        int err_fds[2];
        pipe2(err_fds,O_CLOEXEC);

        pid_t pid = fork();

        if(pid == -1){
            //Fork error
            close(err_fds[0]);
            close(err_fds[1]);
            return {};
        }
        else if(pid == 0){
            execvp(args[0],args);
            //Execute error
            auto err = errno;
            write(err_fds[1],&err,sizeof(err));
            _Exit(-1);
        }
        else{
            //Check has error
            auto handler = std::signal(SIGPIPE,SIG_IGN);
            //Close read
            close(err_fds[1]);
            int error;
            if(read(err_fds[0],&error,sizeof(error)) == sizeof(error)){
                //Has error
                errno = error;
                std::signal(SIGPIPE,handler);
                close(err_fds[0]);
                return {};
            }
            std::signal(SIGPIPE,handler);
            close(err_fds[0]);
            return pid;
        }
    }
    //XError
    XError::XError(const void *_event){
        const XErrorEvent *event = static_cast<const XErrorEvent*>(_event);
        //Get Error String
        char buf[128];
        int ret = XGetErrorText(event->display,event->error_code,buf,sizeof(buf));
        if(ret == -1){
            buf[0] = '\0';
        }
        _x_display = event->display;
        set_message(u8format("%s :%s",XDisplayString(event->display),buf));
    }
    XError::~XError() = default;
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
    void EmbedWindow::nt_set_rect(const Rect &r){
        // if(has_embed()){
        //     //Has window
        //     //Resize and move
        //     XMoveWindow(BTK_X_DISPLAY(_display),BTK_X_WINDOW(_window),x(),y());
        //     XResizeWindow(BTK_X_DISPLAY(_display),BTK_X_WINDOW(_window),w(),h());
        // }
    }
    void EmbedWindow::detach_window(){
        if(has_embed()){
            
        }
    }
    void EmbedWindow::reparent_window(WinPtr w,WinPtr parent,int x,int y){
        XReparentWindow(
            BTK_X_DISPLAY(w.display()),
            BTK_X_WINDOW(w.window()),
            BTK_X_WINDOW(parent.window()),
            x,
            y
        );
        XFlush(BTK_X_DISPLAY(w.display()));
    }
}