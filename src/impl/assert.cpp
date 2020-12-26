#include "../build.hpp"

#include <SDL2/SDL_log.h>

#ifndef NDEBUG
extern "C" void _Btk_ReportFailure(
    const char *file,
    int line,
    const char *fn,
    const char *exp){
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,"Assertion faild on %s:%d %s '%s'",file,line,fn,exp);
    _Btk_Backtrace();
}
#endif