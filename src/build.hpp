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

#ifdef _WIN32
    //Suppress the min and max definitions in Windef.h
    #define NOMINMAX
#endif

//Debug Info
#ifndef NDEBUG
    #define BTK_LOGINFO(...) SDL_Log(__VA_ARGS__)
    #define BTK_LOGWARN(...) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,__VA_ARGS__)
    #define BTK_LOGDEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,__VA_ARGS__)
    #define BTK_DEBUG(...) __VA_ARGS__
#else
    #define BTK_LOGINFO(...)
    #define BTK_LOGWARN(...)
    #define BTK_DEBUG(...)
    #define BTK_LOGDEBUG(...)
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

#define BTK_UNIMPLEMENTED() Btk::throwRuntimeError("unimplemented")

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
     * @brief A struct holded demangled string
     * 
     */
    struct _typeinfo_string{
        _typeinfo_string(const std::type_info &info){
            //TODO
            #ifdef __GNUC__
            str = ::abi::__cxa_demangle(info.name(),nullptr,nullptr,nullptr);
            if(str == nullptr){
                //failed to demangle the name
                str = ::strdup(info.name());
            }
            #else
            str = info.name();
            #endif
        }
        _typeinfo_string(const _typeinfo_string &s){
            #ifdef __GNUC__
            str = ::strdup(s.str);
            #else
            str = s.str;
            #endif
        }
        _typeinfo_string(_typeinfo_string &&s){
            str = s.str;
            s.str = nullptr;
        }
        ~_typeinfo_string(){
            #ifdef __GNUC__
            ::free(const_cast<char*>(str));
            #endif
        }
        const char *str;
        operator u8string() const noexcept{
            return str;
        }
        operator u8string_view() const noexcept{
            return str;
        }
        const char *c_str() const noexcept{
            return str;
        }
        const char *data() const noexcept{
            return str;
        }
    };
    /**
     * @brief Get the typename of a type
     * 
     * @note It usually used in debugging
     * @param info The typeinfo
     * @return The name of the typeinfo(no demangled)
     */
    inline _typeinfo_string get_typename(const std::type_info &info){
        return info;
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
        // int strsize;

        // //Get the size of the string
        // va_list varg;
        // va_start(varg,fmt);
        // #ifdef _WIN32
        // strsize = _vscprintf(fmt,varg);
        // #else
        // strsize = vsnprintf(nullptr,0,fmt,varg);
        // #endif
        // va_end(varg);
        
        // u8string str;
        // str.resize(strsize);

        // //start formatting
        // va_start(varg,fmt);
        // vsprintf(&str[0],fmt,varg);
        // va_end(varg);

        va_list varg;
        va_start(varg,fmt);
        u8string s = u8vformat(fmt,varg);
        va_end(varg);
        return s;
    }
    /**
     * @brief Append text to the string
     * 
     * @param str The container
     * @param fmt The c-style fmt string
     * @param ... The args you want to format
     */
    inline void cformat(u8string &str,const char *fmt,...){
        // int strsize;
        // //Get the size of the string
        // va_list varg;
        // va_start(varg,fmt);
        // #ifdef _WIN32
        // strsize = _vscprintf(fmt,varg);
        // #else
        // strsize = vsnprintf(nullptr,0,fmt,varg);
        // #endif
        // va_end(varg);
        // //Get the '\0'
        // size_t length = str.length();
        
        // str.resize(strsize + str.size());
        // char *end = &str[length];
        // //start formatting
        // va_start(varg,fmt);
        // vsprintf(end,fmt,varg);
        // va_end(varg);
        
        va_list varg;
        va_start(varg,fmt);
        str.append_vfmt(fmt,varg);
        va_end(varg);
    }
};
#endif // _BTK_BUILD_HPP_
