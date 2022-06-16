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

#ifdef _BTK_PRECOMPILED_HEADER
    #include <Btk/signal/bind.hpp>
    #include <Btk/widget.hpp>
    #include <Btk/render.hpp>
    #include <Btk/event.hpp>
#endif


#if BTK_GCC
    //GCC
    #include <strings.h>
    #include <cxxabi.h>
#endif

#if BTK_WIN32
    //Suppress the min and max definitions in Windef.h
    #define NOMINMAX
#endif

//Debug Info
#ifndef NDEBUG
    #define BTK_LOGINFO(...) _BtkL_Info(__VA_ARGS__)
    #define BTK_LOGWARN(...) _BtkL_Warn(__VA_ARGS__)
    #define BTK_LOGDEBUG(...) _BtkL_Info(__VA_ARGS__)
    //< This macro is deprecated
    #define BTK_DEBUG(...) __VA_ARGS__
#else
    #define BTK_LOGINFO(...)
    #define BTK_LOGWARN(...)
    #define BTK_DEBUG(...)
    #define BTK_LOGDEBUG(...)
#endif
//Color
#define BTK_CRED(STR) "\033[31m" STR "\033[0m"
#define BTK_CGREEN(STR) "\033[32m" STR "\033[0m"
#define BTK_CYELLOW(STR) "\033[33m" STR "\033[0m"
#define BTK_CBLUE(STR) "\033[34m" STR "\033[0m"
#define BTK_CMAGENTA(STR) "\033[35m" STR "\033[0m"
//Assert / PANIC
#ifndef NDEBUG
    #define BTK_ASSERT(EXP) if(not(EXP)){\
        _Btk_ReportFailure(__FILE__,__LINE__,BTK_FUNCTION,#EXP);\
        SDL_TriggerBreakpoint();\
    }
#else
    #define BTK_ASSERT(EXP) 
#endif

#ifndef NDEBUG
    #define BTK_PANIC(MSG) BTK_ASSERT(!(MSG))
#elif BTK_GCC
    #define BTK_PANIC(msg) __builtin_trap()
#else
    #define BTK_PANIC(MSG) std::abort()
#endif

//Check type macro
#define BTK_ASSERT_CASTABLE(TYPE,PTR) BTK_ASSERT(dynamic_cast<TYPE*>(PTR) != nullptr)
//Get type macro
#define BTK_typenameof(V) Btk::get_typename(V).c_str()


//Unimpl
#define BTK_UNIMPLEMENTED() Btk::throwRuntimeError(\
    Btk::u8format("TODO require impl at %s:%d fn %s",__FILE__,__LINE__,BTK_FUNCTION)\
)
#define BTK_FIXME(MSG) \
    {\
        static bool done = false;\
        if(not done){\
            fprintf(stderr,"FIXME:%s %s:%s:%d\n",MSG,BTK_FUNCTION,__FILE__,__LINE__);\
            done = true;\
        }\
    }

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
