#if !defined(_BTK_BUILD_HPP_)
#define _BTK_BUILD_HPP_
//Is sourse
#define _BTK_SOURCE

//Debug Info
#ifndef NDEBUG
    #define BTK_LOGINFO(FMT,...) SDL_Log(FMT,__VA_ARGS__)
#else
    #define BTK_LOGINFO(FMT,...)
#endif

#endif // _BTK_BUILD_HPP_
