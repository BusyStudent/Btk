#include "../../build.hpp"

#include <Btk/utils/template.hpp>
#include <Btk/platform/win32.hpp>
#include <Btk/msgbox/msgbox.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/string.hpp>
#include <Btk/Btk.hpp>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL.h>
#include <csignal>
#include <string>

#include <ShlObj.h>

#include "internal.hpp"

#ifndef NDEBUG

#ifdef _MSC_VER
//In msvc we can use StackWalker
#include "../../libs/StackWalker.h"
namespace{
    //Format strng to text
    class WalkerToString:public StackWalker{
        public:
            WalkerToString(Btk::u8string &s):msg(s){

            }
            virtual void OnCallstackEntry(CallstackEntryType,CallstackEntry &addr){
                if(addr.lineNumber == 0){
                    //No line number
                    Btk::cformat(msg,"  at %p: %s (in %s)\n",
                        reinterpret_cast<void*>(addr.offset),
                        addr.name,
                        addr.moduleName);
                }
                else{
                    Btk::cformat(msg,"  at %p: %s (in %s:%d)\n",
                        reinterpret_cast<void*>(addr.offset),
                        addr.name,
                        addr.lineFileName,
                        int(addr.lineNumber));
                }
            }
        Btk::u8string &msg;
    };
}

#endif

static void crash_handler(int sig){
    signal(sig,SIG_DFL);
    const char *signame = "???";
    if(sig == SIGABRT){
        signame = "SIGABRT";
    }
    else if(sig == SIGSEGV){
        signame = "SIGSEGV";
    }
    _Btk_Backtrace();
    Btk::u8string msg = Btk::cformat("Error: Caught signal %s",signame);

    #ifdef _MSC_VER
    msg += "\nCurrent CallStack =>\n";
    static WalkerToString ws(msg);
    ws.ShowCallstack();
    #endif

    if(MessageBoxA(nullptr,msg.c_str(),"Error",MB_ABORTRETRYIGNORE | MB_ICONERROR) == IDABORT){
        TerminateProcess(GetCurrentProcess(),EXIT_FAILURE);
    }
}
/**
 * @brief Get the thread name object
 * 
 * @return wchar_t* (use Locale Free)
 */
static wchar_t *get_thread_name(){
    wchar_t *buf = nullptr;

    HRESULT (*WINAPI get_th_name)(HANDLE thread,PWSTR *);

    HMODULE hmodule = GetModuleHandleA("kernel32.dll");
    get_th_name = (decltype(get_th_name))GetProcAddress(hmodule,"GetThreadDescription");
    if(get_th_name == nullptr){
        return nullptr;
    }
    HRESULT hr = get_th_name(GetCurrentThread(),&buf);
    if(not SUCCEEDED(hr)){
        return nullptr;
    }
    return buf;
}
static LONG CALLBACK seh_exception_handler(_EXCEPTION_POINTERS *exp){
    fprintf(stderr,"Exception at address %p\n",exp->ExceptionRecord->ExceptionAddress);

    wchar_t *th_name = get_thread_name();
    fwprintf(stderr,L"ThreadID:%d name:%s\n",int(GetCurrentThreadId()),th_name);
    LocalFree(th_name);
    
    _Btk_Backtrace();

    fflush(stderr);
    
    Btk::u8string msg = Btk::cformat("Exception at address %p\n",exp->ExceptionRecord->ExceptionAddress);

    #ifdef _MSC_VER
    msg += "\nCurrent CallStack =>\n";
    static WalkerToString ws(msg);
    ws.ShowCallstack();
    #endif
    int box_ret = MessageBoxA(nullptr,msg.c_str(),"Exception",MB_ABORTRETRYIGNORE | MB_ICONERROR);
    if(box_ret == IDABORT){
        //abort it
        TerminateProcess(GetCurrentProcess(),EXIT_FAILURE);
    }
    else if(box_ret == IDIGNORE){
        //ignore it
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
#endif

namespace Btk{
namespace Win32{
    //For slove WM_SIZING
    static ObjectHolder<std::map<HWND,WindowImpl*>> hwnd_map;
    static HIMC ime_handle = nullptr;

    static HWND get_hwnd(SDL_Window *win){
        SDL_SysWMinfo info;
        SDL_GetVersion(&info.version);
        SDL_GetWindowWMInfo(win,&info);
        return info.info.win.window;
    }
    static void on_add_window(WindowImpl *win){
        HWND h = get_hwnd(win->sdl_window());
        hwnd_map->emplace(std::make_pair(h,win));

        BTK_LOGINFO("[System::Win32]Reginster window %p HWND %p",win,h);
    }
    static void on_del_window(WindowImpl *win){
        HWND h = get_hwnd(win->sdl_window());
        hwnd_map->erase(hwnd_map->find(h));

        BTK_LOGINFO("[System::Win32]Remove window %p HWND %p", win, h);
    }
    static int SDLCALL event_filter(void *,SDL_Event *event){
        if(event->type == SDL_SYSWMEVENT){
            //Handle it right now
            HandleSysMsg(*(event->syswm.msg));
            return 0;
        }
        return 1;
    }
    void Init(){
        #ifndef NDEBUG
        signal(SIGABRT,crash_handler);
        signal(SIGSEGV,crash_handler);
        signal(SIGTERM,crash_handler);
        //Debug show the error
        SetUnhandledExceptionFilter(seh_exception_handler);

        #endif

        CoInitializeEx(nullptr,COINIT_MULTITHREADED);
        //Init IME
        ime_handle = ImmCreateContext();
        //Init map
        hwnd_map.construct();

        //Bind to register
        Instance().signal_window_created.connect(on_add_window);
        Instance().signal_window_closed.connect(on_del_window);

        SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
        SDL_SetEventFilter(event_filter,nullptr);
    }
    void Quit(){
        CoUninitialize();

        //Delete map
        SDL_EventState(SDL_SYSWMEVENT,SDL_DISABLE);
        SDL_SetEventFilter(nullptr,nullptr);
        hwnd_map.destroy();

        ImmDestroyContext(ime_handle);
    }
    void HandleSysMsg(const SDL_SysWMmsg &msg){
        auto info = msg.msg.win;
        WindowImpl *win = GetWindow(info.hwnd);
        if(win != nullptr){
            return win->handle_win32(
                info.hwnd,
                info.msg,
                info.wParam,
                info.lParam
            );
        }
    }
    u8string StrMessageA(DWORD errcode){
        char16_t *ret;
        DWORD result = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            errcode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&ret),
            0,
            nullptr
        );
        if(result == 0){
            throwWin32Error();
        }
        u8string s;
        Utf16To8(s,ret);
        LocalFree(ret);
        return s;
    }
    u16string StrMessageW(DWORD errcode){
        char16_t *ret;
        DWORD result = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            errcode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPWSTR>(&ret),
            0,
            nullptr
        );
        if(result == 0){
            throwWin32Error();
        }
        u16string s(ret);
        LocalFree(ret);
        return s;
    }
    bool MessageBox(std::string_view title,std::string_view msg,int flag){
        UINT type = 0;

        if(flag == MessageBox::Info){
            type |= MB_ICONINFORMATION;
        }
        else if(flag == MessageBox::Warn){
            type |= MB_ICONWARNING;
        }
        else if(flag == MessageBox::Error){
            type |= MB_ICONERROR;
        }
        MessageBoxW(
            GetForegroundWindow(),
            reinterpret_cast<const wchar_t*>(Utf8To16(msg).c_str()),
            reinterpret_cast<const wchar_t*>(Utf8To16(title).c_str()),
            type
        );
        return true;
    }
    WindowImpl *GetWindow(HWND h){
        auto iter = hwnd_map->find(h);
        if(iter == hwnd_map->end()){
            return nullptr;
        }
        return iter->second;
    }
}
}
namespace Btk{
    Win32Error::Win32Error(DWORD errcode):
        std::runtime_error(Win32::StrMessageA(errcode).c_str()){

    }
    Win32Error::~Win32Error(){

    }
    //GetErrorCode
    const char *Win32Error::what() const noexcept{
        try{
            return FillInternalU8Buffer(Win32::StrMessageA(errcode)).c_str();
        }
        catch(...){
            //Formatting error or etc....
            return "<Unknown>";
        }
    }
    [[noreturn]] void throwWin32Error(DWORD errcode){
        throw Win32Error(errcode);
    }
    bool HideConsole(){
        return FreeConsole();
    }
}
namespace Btk{
    void WindowImpl::handle_win32(
        void *hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam){
        
        //Process Win32 Evennt
        switch(message){
        case WM_SIZING:{
                RECT *rect = reinterpret_cast<RECT *>(lParam);

                SDL_Event event;
                event.type = SDL_WINDOWEVENT;
                event.window.windowID = id();
                event.window.timestamp = SDL_GetTicks();
                event.window.event = SDL_WINDOWEVENT_RESIZED;
                event.window.data1 = rect->right - rect->left;
                event.window.data2 = rect->bottom - rect->top;
                try{
                    handle_windowev(event);
                    //Limit to draw
                    auto current = SDL_GetTicks();
                    //Update it

                    if (fps_limit > 0){
                        //Has limit
                        Uint32 durl = 1000 / fps_limit;
                        if (current - win32_draw_ticks < durl){
                            //Too fast,ignore it
                            BTK_LOGINFO("Drawing too fast,ignored");
                            return;
                        }
                    }
                    win32_draw_ticks = current;
                    //Execute draw right now
                    draw();
                }
                catch (...){
                    DeferCall(std::rethrow_exception, std::current_exception());
                }
                break;
            }
        }
    }
}