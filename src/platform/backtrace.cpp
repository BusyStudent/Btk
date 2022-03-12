#include "../build.hpp"

#include <Btk/platform/popen.hpp>
#ifndef NDEBUG
#include <string_view>
#include <string>

#if __has_include(<unwind.h>)
//We could use unwind
#include <unwind.h>
#include <vector>
namespace BtkUnwind{
    //...
    using Input = std::vector<void*>;
    inline auto GetCallStack() -> Input{
        //Backtrace function
        auto fn = [](struct _Unwind_Context *uc, void *p) -> _Unwind_Reason_Code{
            //...
            Input *input = static_cast<Input*>(p);
            auto pc = _Unwind_GetIP(uc);
            if(pc == 0){
                return _URC_END_OF_STACK;
            }
            input->push_back(reinterpret_cast<void*>(pc));
            return _URC_NO_REASON;
        };
        Input input;
        _Unwind_Backtrace(fn,&input);
        //Remove self
        input.erase(input.begin());
        return input;
    }
}
#endif

//GNU Linux impl
#ifdef __gnu_linux__
#include <execinfo.h>
#include <unistd.h>
#include <cxxabi.h>
#include <cstdio>
using abi::__cxa_demangle;
extern "C" void _Btk_Backtrace(){
    void *address[100];
    int ret = backtrace(address,100);

    fprintf(stderr,"Backtrace Begin pid:%d\n",int(getpid()));
    //get address
    char **str = backtrace_symbols(address,ret);
    if(str == nullptr){
        fputs("  Error :Fail to get symbols\n",stderr);
    }
    //Function name to demangle
    std::string funcname;
    //File name
    std::string filename;
    //File symbols end position
    size_t file_end;
    size_t func_end;

    size_t func_plus;//'+' positions

    int demangle_status;

    char *func;
    //Buffer for __cxa_demangle
    char *outbuffer = nullptr;
    size_t outbuflen = 0;
    for(int i = 0;i < ret;i++){
        std::string_view view(str[i]);

        file_end = view.find_first_of('(');
        //Get filename
        filename = view.substr(0,file_end);
        //Get function
        func_end = view.find_first_of(')',file_end + 1);
        func_plus = view.find_first_of('+',file_end + 1);

        if(func_plus > func_end or func_plus == std::string_view::npos){
            //Not found it
            funcname = view.substr(file_end + 1,func_end - file_end);
        }
        else{
            //We cannot get the funcion name
            if(func_plus == file_end + 1){
                funcname = std::string_view("???");
            }
            else{
                funcname = view.substr(file_end + 1,func_plus - file_end - 1);
            }
        }
        //To demangle the symbols
        func = __cxa_demangle(funcname.c_str(),outbuffer,&outbuflen,&demangle_status);
        if(func == nullptr){
            //demangle failed
            //using the raw name
            func = funcname.data();
        }
        else{
            outbuffer = func;
        }
        fprintf(stderr,"  at %p: %s (in %s)\n",address[i],func,filename.c_str());
    }
    free(str);
    free(outbuffer);
    fputs("Backtrace End\n",stderr);
    fflush(stderr);
};
#elif defined(_WIN32) && defined(_MSC_VER)
//Win32 backtrace with msvc
#include <windows.h>
#include <DbgHelp.h>
#include "../libs/StackWalker.h"
#include "../libs/StackWalker.cpp"
namespace{
    /**
     * @brief Output the data to stderr
     * 
     */
    class WalkerToStderr:public StackWalker{
        public:
            virtual void OnCallstackEntry(CallstackEntryType,CallstackEntry &addr){
                if(addr.offset == 0){
                    return;
                }
                if(addr.lineNumber == 0){
                    //No line number
                    fprintf(stderr,"  at %p: %s (in %s)\n",
                        reinterpret_cast<void*>(addr.offset),
                        addr.name,
                        addr.moduleName
                    );
                }
                else{
                    fprintf(stderr,"  at %p: %s (in %s:%d)\n",
                        reinterpret_cast<void*>(addr.offset),
                        addr.name,
                        addr.lineFileName,
                        int(addr.lineNumber));
                }
            }
    };
}
extern "C" void _Btk_Backtrace(){
    static WalkerToStderr walker;

    if(not walker.ShowCallstack()){
        fprintf(stderr,"  Fail to show backtrace\n");
    }
}

#elif BTK_MINGW

#include <Btk/platform/win32.hpp>
#include <dbgeng.h>
#include <cxxabi.h>

__CRT_UUID_DECL(IDebugClient,0x27fe5639,0x8407,0x4f47,0x83,0x64,0xee,0x11,0x8f,0xb0,0x8a,0xc8)
__CRT_UUID_DECL(IDebugControl,0x5182e668,0x105e,0x416e,0xad,0x92,0x24,0xef,0x80,0x04,0x24,0xba)
__CRT_UUID_DECL(IDebugSymbols,0x8c31e98c,0x983a,0x48a5,0x90,0x16,0x6f,0xe5,0xd6,0x67,0xa9,0x50)

#define C_RED FOREGROUND_RED
#define C_GREEN FOREGROUND_GREEN
#define C_BLUE FOREGROUND_BLUE

#define C_YELLOW (C_GREEN | C_RED)
#define C_PURPLE (C_RED | C_BLUE)
#define C_TBLUE  (C_GREEN | C_BLUE)
#define C_WHITE (C_RED | C_GREEN | C_BLUE)

#define SET_COLOR(C) \
    SetConsoleTextAttribute( \
        GetStdHandle(STD_ERROR_HANDLE), \
        C \
    );


static HMODULE dbg_eng = nullptr;
static decltype(DebugCreate) *dbg_crt;

static void init_dbg_help(){
    if(dbg_crt == nullptr){
        dbg_eng = LoadLibraryA("DBGENG.dll");
        if(dbg_eng == nullptr){
            std::abort();
        }
        dbg_crt = (decltype(DebugCreate)*)GetProcAddress(dbg_eng,"DebugCreate");
    }
}

extern "C" void _Btk_Backtrace(){
    auto callstack = BtkUnwind::GetCallStack();

    // DWORD n = RtlCaptureStackBackTrace(
    //     0,
    //     callstack.size(),
    //     callstack.data(),
    //     0
    // );
    // BTK_ASSERT(n == callstack.size());

    init_dbg_help();
    //create debug interface
    Btk::Win32::ComInstance<IDebugClient> client;
    Btk::Win32::ComInstance<IDebugControl> control;
    
    dbg_crt(__uuidof(IDebugClient),reinterpret_cast<void**>(&client));
    client->QueryInterface(__uuidof(IDebugControl),reinterpret_cast<void**>(&control));
    //attach
    client->AttachProcess(
        0,
        GetCurrentProcessId(),
        DEBUG_ATTACH_NONINVASIVE | DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND
    );
    control->WaitForEvent(DEBUG_WAIT_DEFAULT,INFINITE);
    //Get symbols
    Btk::Win32::ComInstance<IDebugSymbols> symbols;
    client->QueryInterface(__uuidof(IDebugSymbols),reinterpret_cast<void**>(&symbols));
    //Done

    //Print Stack
    bool has_fn_line;
    char  buf[256];
    char  file[256];

    ULONG size;
    ULONG  line;

    int naddr = callstack.size();

    SET_COLOR(C_RED);
    fputs("--Backtrace Begin--\n",stderr);
    fflush(stderr);

    for(auto addr:callstack){
        //try get name
        if(symbols->GetNameByOffset(
            reinterpret_cast<ULONG64>(addr),
            buf,
            sizeof(buf),
            &size,
            0
        ) == S_OK){
            buf[size] = '\0';
        }
        else{
            std::strcpy(buf,"???");
        }
        //try find !
        char *n_start = std::strchr(buf,'!');
        if(n_start != nullptr){
            *n_start = '_';
            //To __Znxxx
            //Begin demangle
            char *ret_name = abi::__cxa_demangle(n_start,nullptr,nullptr,nullptr);
            if(ret_name != nullptr){
                //has it name
                strcpy(n_start + 1,ret_name);
                std::free(ret_name);
            }
            *n_start = '!';
        }
        else{
            //didnot has function name
            strcat(buf,"!Unknown");
        }
        //Try get source file
        if(symbols->GetLineByOffset(
            reinterpret_cast<ULONG64>(addr),
            &line,
            file,
            sizeof(file),
            &size,
            0
        ) == S_OK){
            file[size] = '\0';
            has_fn_line = true;
            // fprintf(stderr,"[%d] %s at %s:%d\n",naddr,buf,file,line);
        }
        else{
            has_fn_line = false;
            // fprintf(stderr,"[%d] %s\n",naddr,buf);
        }

        SET_COLOR(C_WHITE);
        fputc('[',stderr);
        fflush(stderr);
        
        SET_COLOR(C_YELLOW);
        //Print number
        fprintf(stderr,"%d",naddr);
        fflush(stderr);

        SET_COLOR(C_WHITE);
        fputc(']',stderr);
        fflush(stderr);

        //Print fn name
        fprintf(stderr," %s",buf);
        fflush(stderr);

        if(has_fn_line){
            fprintf(stderr," at %s:",file);
            fflush(stderr);
            SET_COLOR(C_TBLUE);
            fprintf(stderr,"%d",line);
            fflush(stderr);
        }


        fputc('\n',stderr);
        naddr--;
    }

    SET_COLOR(C_RED);
    fputs("--Backtrace End--\n",stderr);
    fflush(stderr);

    SET_COLOR(C_WHITE);
}

#else
    extern "C" void _Btk_Backtrace(){}
#endif

#endif