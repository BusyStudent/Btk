#include "../../build.hpp"

#include <Btk/platform/win32.hpp>
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
#endif

namespace Btk{
namespace Win32{
    void Init(){
        #ifndef NDEBUG
        signal(SIGABRT,crash_handler);
        signal(SIGSEGV,crash_handler);
        signal(SIGTERM,crash_handler);
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
    [[noreturn]] void throwWin32Error(DWORD errcode){
        throw Win32Error(errcode);
    }
}