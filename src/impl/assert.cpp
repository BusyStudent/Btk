#include "../build.hpp"

#include <SDL2/SDL_log.h>

#ifdef _WIN32
    #include <windows.h>
#endif

#ifndef NDEBUG
extern "C" void _Btk_ReportFailure(
    const char *file,
    int line,
    const char *fn,
    const char *exp){
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,"Assertion faild on %s:%d %s '%s'",file,line,fn,exp);
    
    #ifdef _WIN32
    auto msg = Btk::cformat("Assertion faild on %s:%d %s '%s'",file,line,fn,exp);
    MessageBoxA(nullptr,msg.c_str(),"Assertion Failed",MB_ICONERROR);
    #endif

    _Btk_Backtrace();
}
#endif