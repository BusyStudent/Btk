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
//Assert / PANIC
#ifndef NDEBUG
    #define BTK_ASSERT(EXP) if(not(EXP)){\
        _Btk_ReportFailure(__FILE__,__LINE__,BTK_FUNCTION,#EXP);\
        SDL_TriggerBreakpoint();\
    }
#else
    #define BTK_ASSERT(EXP) 
#endif

//Check type macro
#define BTK_ASSERT_CASTABLE(TYPE,PTR) BTK_ASSERT(dynamic_cast<TYPE*>(PTR) != nullptr)
//Get type macro
#define BTK_typenameof(V) Btk::get_typename(T).c_str()


//Unimpl
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
    T event_cast(U &&u) noexcept{
        return static_cast<T>(std::forward<U>(u));
    }
    /**
     * @brief A struct holded demangled string
     * 
     */
    struct _typeinfo_string{
        _typeinfo_string(const std::type_info &info) noexcept{
            //TODO
            #ifdef __GNUC__
            str = ::abi::__cxa_demangle(info.name(),nullptr,nullptr,nullptr);
            if(str == nullptr){
                //failed to demangle the name
                str = info.name();
                need_free = false;
            }
            else{
                need_free = true;
            }
            #else
            str = info.name();
            #endif
        }
        _typeinfo_string(const _typeinfo_string &s) noexcept{
            #ifdef __GNUC__
            if(s.need_free){
                //allocate in heap
                str = ::strdup(s.str);
            }
            else{
                str = s.str;
            }
            need_free = s.need_free;
            #else
            str = s.str;
            #endif
        }
        _typeinfo_string(_typeinfo_string &&s) noexcept{
            str = s.str;
            s.str = nullptr;
            #ifdef __GNUC__
            need_free = true;
            s.need_free = false;
            #endif
        }
        ~_typeinfo_string() noexcept{
            #ifdef __GNUC__
            if(need_free){
                ::free(const_cast<char*>(str));
            }
            #endif
        }
        //Members
        const char *str;
        #ifdef __GNUC__
        bool need_free;
        #endif

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
        size_t size() const noexcept{
            return ::strlen(str);
        }
    };
    /**
     * @brief Get the typename of a type
     * 
     * @note It usually used in debugging
     * @param info The typeinfo
     * @return The name of the typeinfo(no demangled)
     */
    inline _typeinfo_string get_typename(const std::type_info &info) noexcept{
        return info;
    }
    template<class T>
    inline _typeinfo_string get_typename(const T *ptr){
        return get_typename(typeid(*ptr));
    }
}
#endif // _BTK_BUILD_HPP_
