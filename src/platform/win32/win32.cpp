#include "../../build.hpp"

#include <Btk/platform/win32.hpp>
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
    }
    void Quit(){
        CoUninitialize();
    }
}
}