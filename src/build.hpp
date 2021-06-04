#if !defined(_BTK_BUILD_HPP_)
#define _BTK_BUILD_HPP_
//For Trigger BreakPoint
#include <SDL2/SDL_assert.h>
#include <SDL2/SDL_log.h>
#include <typeinfo>
#include <utility>
#include <cstdarg>
#include <cstring>
#include <string>
//Is sourse
#define _BTK_SOURCE
#include <Btk/defs.hpp>
#include <Btk/string.hpp>

#if defined(_MSC_VER)
    //MSVC
    #define BTK_FUNCTION __FUNCSIG__
#elif defined(__GNUC__)
    //GCC
    #include <strings.h>
    #include <cxxabi.h>
    #define BTK_FUNCTION __PRETTY_FUNCTION__
#else
    #define BTK_FUNCTION SDL_FUNCTION
#endif
//Debug Info
#ifndef NDEBUG
    #define BTK_LOGINFO(...) SDL_Log(__VA_ARGS__)
    #define BTK_LOGWARN(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,__VA_ARGS__)
    #define BTK_DEBUG(...) __VA_ARGS__
#else
    #define BTK_LOGINFO(...)
    #define BTK_LOGWARN(...)
    #define BTK_DEBUG(...)
#endif
//Assert
#ifndef NDEBUG
    #define BTK_ASSERT(EXP) if(not(EXP)){\
        _Btk_ReportFailure(__FILE__,__LINE__,BTK_FUNCTION,#EXP);\
        SDL_TriggerBreakpoint();\
    }
#else
    #define BTK_ASSERT(EXP) 
#endif



#ifndef NDEBUG
/**
 * @brief Show the backtrace
 * @note This function only avliabled on debug version
 */
extern "C" void BTKAPI _Btk_Backtrace();
/**
 * @brief Report Assertion failure
 * @note This function only avliabled on debug version
 */
extern "C" void BTKAPI _Btk_ReportFailure(
    const char *file,
    int line,
    const char *fn,
    const char *exp
);
#else
extern "C" inline void _Btk_Backtrace(){};
extern "C" inline void _Btk_ReportFailure(
    const char *file,
    int line,
    const char *fn,
    const char *exp
){};
#endif

namespace Btk{
    //Cast event for debugging
    template<class T,class U>
    T event_cast(U &&u){
        #ifndef NDEBUG
        return dynamic_cast<T>(std::forward<U>(u));
        #else
        return static_cast<T>(std::forward<U>(u));
        #endif
    }
    inline int vscprintf(const char *fmt,va_list varg){
        #ifdef _WIN32
        return _vscprintf(fmt,varg);
        #else
        return vsnprintf(nullptr,0,fmt,varg);
        #endif
    };
    /**
     * @brief Get the typename of a type
     * 
     * @note It usually used in debugging
     * @param info The typeinfo
     * @return The name of the typeinfo(no demangled)
     */
    inline u8string get_typename(const std::type_info &info){
        #ifdef __GNUC__
        char *ret = abi::__cxa_demangle(info.name(),nullptr,nullptr,nullptr);
        if(ret == nullptr){
            //failed to demangle the name
            return info.name();
        }
        else{
            u8string name(ret);
            free(ret);
            return name;
        }
        #else
        return info.name();
        #endif
    }
    template<class T>
    inline u8string get_typename(const T *ptr){
        return get_typename(typeid(*ptr));
    }
    /**
     * @brief Using c-syle formatting
     * 
     * @param fmt The c-style fmt string
     * @param ... The args you want to format
     * @return std::string 
     */
    inline u8string cformat(const char *fmt,...){
        int strsize;

        //Get the size of the string
        va_list varg;
        va_start(varg,fmt);
        #ifdef _WIN32
        strsize = _vscprintf(fmt,varg);
        #else
        strsize = vsnprintf(nullptr,0,fmt,varg);
        #endif
        va_end(varg);
        
        u8string str;
        str.resize(strsize);

        //start formatting
        va_start(varg,fmt);
        vsprintf(&str[0],fmt,varg);
        va_end(varg);

        return str;
    }
    /**
     * @brief Append text to the string
     * 
     * @param str The container
     * @param fmt The c-style fmt string
     * @param ... The args you want to format
     */
    inline void cformat(u8string &str,const char *fmt,...){
        int strsize;
        //Get the size of the string
        va_list varg;
        va_start(varg,fmt);
        #ifdef _WIN32
        strsize = _vscprintf(fmt,varg);
        #else
        strsize = vsnprintf(nullptr,0,fmt,varg);
        #endif
        va_end(varg);
        //Get the '\0'
        size_t length = str.length();
        
        str.resize(strsize + str.size());
        char *end = &str[length];
        //start formatting
        va_start(varg,fmt);
        vsprintf(end,fmt,varg);
        va_end(varg);
    }
};
#endif // _BTK_BUILD_HPP_
