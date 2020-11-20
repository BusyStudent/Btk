#if !defined(_BTK_BUILD_HPP_)
#define _BTK_BUILD_HPP_
//For Trigger BreakPoint
#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_log.h>
//Is sourse
#define _BTK_SOURCE

#if defined(_MSC_VER)
    //MSVC
    #define BTK_FUNCTION __FUNCSIG__
#elif defined(__GNUC__)
    //GCC
    #define BTK_FUNCTION __PRETTY_FUNCTION__
#else
    #define BTK_FUNCTION SDL_FUNCTION
#endif

//Debug Info
#ifndef NDEBUG
    #define BTK_LOGINFO(FMT,...) SDL_Log(FMT,__VA_ARGS__)
#else
    #define BTK_LOGINFO(FMT,...)
#endif
//Assert
#ifndef NDEBUG
    #define BTK_ASSERT(EXP) if(not(EXP)){\
        SDL_LogCritical(\
            SDL_LOG_CATEGORY_APPLICATION,"Assertion faild on %s:%d %s '%s'",\
            __FILE__,__LINE__,BTK_FUNCTION,#EXP);\
        SDL_TriggerBreakpoint();\
    }
#else
    #define BTK_ASSERT(EXP) (EXP)
#endif
#endif // _BTK_BUILD_HPP_
