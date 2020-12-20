#include "../build.hpp"

#ifndef NDEBUG
#include <string_view>
#include <string>
//GNU Linux impl
#ifdef __gnu_linux__
#include <execinfo.h>
#include <unistd.h>
#include <cxxabi.h>
#include <cstdio>
using __cxxabiv1::__cxa_demangle;
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
        func = __cxa_demangle(funcname.c_str(),nullptr,nullptr,&demangle_status);
        if(func == nullptr){
            //demangle failed
            //using the raw name
            func = funcname.data();
        }
        fprintf(stderr,"  at %p: %s (in %s)\n",address[i],func,filename.c_str());
        //Is the data from malloc
        if(func != funcname.data()){
            free(func);
        }

    }
    free(str);
    fputs("Backtrace End\n",stderr);
    fflush(stderr);
};
#else
    extern "C" void _Btk_Backtrace(){}
#endif

#endif