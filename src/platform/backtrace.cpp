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
            input->push_back(reinterpret_cast<void*>(_Unwind_GetIP(uc)));
            return _URC_NO_REASON;
        };
        Input input;
        _Unwind_Backtrace(fn,&input);
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

#else
    extern "C" void _Btk_Backtrace(){}
#endif

#endif