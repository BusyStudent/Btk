#include "../../build.hpp"

#include <Btk/platform/win32.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL.h>
#include <csignal>
#include <string>

#include <ShlObj.h>

#ifndef NDEBUG

#ifdef _MSC_VER
//In msvc we can use StackWalker
#include "../../libs/StackWalker.h"
namespace{
    //Format strng to text
    class WalkerToString:public StackWalker{
        public:
            WalkerToString(std::string &s):msg(s){

            }
            virtual void OnCallstackEntry(CallstackEntryType,CallstackEntry &addr){
                msg += Btk::cformat("  at %p: %s (in %s)\n",
                    reinterpret_cast<void*>(addr.offset),
                    addr.name,
                    addr.moduleName);
            }
        std::string &msg;
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
    std::string msg = Btk::cformat("Error: Caught signal %s",signame);

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
    
    std::string msg = Btk::cformat("Exception at address %p\n",exp->ExceptionRecord->ExceptionAddress);

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
    void Init(){
        #ifndef NDEBUG
        signal(SIGABRT,crash_handler);
        signal(SIGSEGV,crash_handler);
        signal(SIGTERM,crash_handler);
        //Debug show the error
        SetUnhandledExceptionFilter(seh_exception_handler);

        #endif

        CoInitializeEx(nullptr,COINIT_MULTITHREADED);
        //SDL_EventState(SDL_SYSWMEVENT,SDL_ENABLE);
    }
    void Quit(){
        CoUninitialize();
    }
    void HandleSysMsg(const SDL_SysWMmsg &msg){

    }
    std::string StrMessageA(DWORD errcode){
        char *ret;
        DWORD result = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            errcode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&ret),
            0,
            nullptr
        );
        if(result == 0){
            throwWin32Error();
        }
        std::string s(ret);
        LocalFree(ret);
        return s;
    }
    std::u16string StrMessageW(DWORD errcode){
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
        std::u16string s(ret);
        LocalFree(ret);
        return s;
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
}