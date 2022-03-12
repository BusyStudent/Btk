#include "../../build.hpp"

#include <Btk/gl/opengl_adapter.hpp>
#include <Btk/platform/popen.hpp>
#include <Btk/platform/alloca.hpp>
#include <Btk/platform/x11.hpp>
#include <Btk/platform/fs.hpp>
#include <Btk/detail/window.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/detail/core.hpp>
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

#include <GL/glx.h>

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

//TODO use XGetDeafults to get system infomation

namespace{
    static int visual_attribs[] = {
      GLX_X_RENDERABLE    , True,
      GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
      GLX_RENDER_TYPE     , GLX_RGBA_BIT,
      GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
      GLX_RED_SIZE        , 8,
      GLX_GREEN_SIZE      , 8,
      GLX_BLUE_SIZE       , 8,
      GLX_ALPHA_SIZE      , 8,
      GLX_DEPTH_SIZE      , 24,
      GLX_STENCIL_SIZE    , 8,
      GLX_DOUBLEBUFFER    , True,
      GLX_SAMPLE_BUFFERS  , 1,
      GLX_SAMPLES         , 4,
      None
    };
    struct GLX:public Btk::GLAdapter{
        ::Display *  display = {};
        //Current
        ::Window     window = {};
        ::GLXContext context = {};
        //Prev
        ::Window     prev_window = {};
        ::GLXContext prev_context = {};

        ~GLX(){

        }
        void   initialize(void *win_handle){

        }
        //Env
        void  *get_proc(const char *name){

        }
        void   get_drawable(int *w,int *h){

        }
        void   get_window_size(int *w,int *h){

        }
        bool   has_extension(const char *extname){

        }
        void   swap_buffer(){
            glXSwapBuffers(display,window);
        }

        void   begin_context(){
            prev_context = glXGetCurrentContext();
            if(prev_context == context){
                //We needed set back
                prev_window = XNone;
            }
            else{
                prev_window = glXGetCurrentDrawable();
                //Set up now
                glXMakeCurrent(display,window,context);
            }
        }
        void   end_context(){
            if(prev_window != XNone){
                glXMakeCurrent(display,prev_window,prev_context);
                prev_window = XNone;
            }
        }
        void   make_current() {
            glXMakeCurrent(display,window,context);
        }
    };
}


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
    static XDisplay *x_display = nullptr;

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
    void *GetXDisplay(){
        if(x_display != nullptr){
            return x_display;
        }
        SDL_Window *win = SDL_CreateWindow(
            nullptr,
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            100,
            100,
            SDL_WINDOW_HIDDEN
        );
        if(win == nullptr){
            throwSDLError();
        }
        auto [display,window] = GetXContext(win);

        SDL_DestroyWindow(win);

        x_display = display;
        return x_display;
    }

    void Init(){
        //Set SDL Hit
        //Disable the compositor
        //Beacuse it will cause a render error in KDE
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR,"0");
        #if 1 && defined(SDL_HINT_VIDEO_X11_WINDOW_VISUALID)
        //Set visual ID to 32 depth if could
        auto display = static_cast<XDisplay*>(GetXDisplay());
        XVisualInfo vinfo;
        if(XMatchVisualInfo(display,DefaultScreen(display),32,TrueColor,&vinfo)){
            SDL_SetHint(SDL_HINT_VIDEO_X11_WINDOW_VISUALID,std::to_string(vinfo.visualid).c_str());
        }
        #endif

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
    SDL_Window *CreateTsWindow(u8string_view title,int h,int w,WindowFlags){
        XDisplay *display = static_cast<XDisplay*>(GetXDisplay());
        XWindow   win;
        
        XVisualInfo vinfo;
        XMatchVisualInfo(display, DefaultScreen(display), 32, TrueColor, &vinfo);

        XSetWindowAttributes attr;
        attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
        attr.border_pixel = 0;
        attr.background_pixel = 0;

        win = XCreateWindow(display, DefaultRootWindow(display), 0, 0, w,h, 0, vinfo.depth, InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);
        XSelectInput(display, win, StructureNotifyMask);

        Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", 0);
        XSetWMProtocols(display, win, &wm_delete_window, 1);

        //Setup opengl
        
        //X11 End
        SDL_Window *sdl = SDL_CreateWindowFrom(reinterpret_cast<void*>(win));

        SDL_SetWindowData(sdl,"btk_x11",0);
        SDL_SetWindowTitle(sdl,title.data());

        return sdl;
    }
    bool GetSystemColor(ColorType t,Color &c){
        const char *opt = nullptr;
        const char *prog = "Text";
        switch(t){
            case BackgroundColor:
                opt = "background";
                break;
            case ForegroundColor:
                opt = "foreground";
                break;
            case SelectionBackgroundColor:
                opt = "selectBackground";
                break;
        }

        XColor xcolor;
        XDisplay *display = static_cast<XDisplay*>(GetXDisplay());
        char *ret = XGetDefault(display,prog,opt);

        if(XParseColor(display,DefaultColormap(display,0),ret,&xcolor) != 0){            
            c = {xcolor.red >> 8,xcolor.green >> 8,xcolor.blue >> 8};
            return true;
        }
        return false;
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
            args[i] = Btk_SmallStrndup(data.str,data.n);
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
            //Cleanup array
            for(i = 0;i < argc;i++){
                Btk_SmallFree(args[i]);
            }

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
    //Exec
    FILE *_vpopen(size_t nargs,const _vspawn_arg arr[]){

        //Allocate args array
        char **args = static_cast<char**>(alloca((nargs + 1) * sizeof(char*)));
        size_t i;
        for(i = 0;i < nargs;i++){
            auto &data = arr[i];
            args[i] = static_cast<char*>(alloca(data.n + 1));
            memcpy(args[i],data.str,data.n);
            args[i][data.n] = '\0';
        }
        args[nargs] = nullptr;

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

                errno = error;
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
}