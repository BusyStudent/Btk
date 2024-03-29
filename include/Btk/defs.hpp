#if !defined(_BTK_DEFS_HPP_)
#define _BTK_DEFS_HPP_
//Get complier / platform
#include "platform/macro.hpp"
#include "detail/config.hpp"

//Export symbols
#ifdef _BTK_SOURCE
    //source file
    #define BTKAPI BTKEXPORT
#else
    //#define BTKAPI BTKIMPORT
    #define BTKAPI
#endif

#if BTK_CXX
    #include <cstdint>
    #include <cstring>
    //For C functions
    #define BTK_CEXTERN extern "C"
    #define BTK_CDECLS_BEGIN extern "C"{
    #define BTK_CDECLS_END }
#else
    #include <stdint.h>
    #include <string.h>
    #define BTK_CEXTERN
    #define BTK_CDECLS_BEGIN
    #define BTK_CDECLS_END
#endif

//Namespace
#ifndef BTK_NAMESPACE
    #define BTK_NAMESPACE Btk
#endif

#define BTK_NS_BEGIN(X) namespace X{
#define BTK_NS_END }
//Unique name
#define BTK_MARGE_TOKEN_IMPL(A,B) A ## B
#define BTK_MARGE_TOKEN(A,B) BTK_MARGE_TOKEN_IMPL(A,B)
#ifdef __COUNTER__
    #define BTK_UNIQUE_NAME(NAME) BTK_MARGE_TOKEN(NAME,__COUNTER__)
#else
    #define BTK_UNIQUE_NAME(NAME) BTK_MARGE_TOKEN(NAME,__LINE__)
#endif
//Stringify
#define BTK_STRINGIFY(A) #A
//Noexcept
#define BTK_NOEXCEPT_IF(X) noexcept(noexcept(X))

/**
 * @brief Generate operator for enum
 * @param ENUM The enum type
 * @param BASE The enum base type
 * @param OP The operator you want to impl
 */
#define BTK_ENUM_OPERATOR(ENUM,BASE,OP) \
    inline ENUM operator OP(ENUM a1,ENUM a2) noexcept{\
        return static_cast<ENUM>(\
            static_cast<BASE>(a1) OP static_cast<BASE>(a2)\
        );\
    }
#define BTK_ENUM_OPERATOR2(ENUM,BASE,OP) \
    inline ENUM operator OP##=(ENUM &a1,ENUM a2) noexcept{\
        a1 = static_cast<ENUM>(\
            static_cast<BASE>(a1) OP static_cast<BASE>(a2)\
        );\
        return a1;\
    }
#define BTK_ENUM_ALIAS(ENUM,ALIAS,OP) \
    inline ENUM operator ALIAS(ENUM a1,ENUM a2) noexcept{\
        return a1 OP a2;\
    }
/**
 * @brief Generate flags' operators
 * 
 */
#define BTK_FLAGS_OPERATOR(ENUM,BASE) \
    BTK_ENUM_OPERATOR(ENUM,BASE, &);\
    BTK_ENUM_OPERATOR(ENUM,BASE, |);\
    BTK_ENUM_OPERATOR(ENUM,BASE, ^);\
    BTK_ENUM_OPERATOR2(ENUM,BASE, &);\
    BTK_ENUM_OPERATOR2(ENUM,BASE, |);\
    BTK_ENUM_OPERATOR2(ENUM,BASE, ^);\
    BTK_ENUM_ALIAS(ENUM,+,|);\
    BTK_ENUM_ALIAS(ENUM,+=,|=);

#if defined(BTK_SHADOW_UNSAFED_HINT) && !defined(DOXYGEN_SHOULD_SKIP_THIS)
    //< Only for shadow parent method
    struct _Btk_shadow_t;
    //Shadow message
    #define BTK_SHADOWED_MESSAGE(NAME) \
        "This " #NAME " method is shadowed explicitly."\
        "If you really know what you are doing,"\
        "you can use OBJ->ParentClass::" #NAME " instead,but it is not recommended"
    //Shadow parent method if you don't want user to use it
    #define BTK_SHADOW_METHOD(NAME) \
        template<class T = void,class ...Args>\
        int NAME(Args ...) noexcept{\
            static_assert(\
                std::is_same_v<T,_Btk_shadow_t>,\
                BTK_SHADOWED_MESSAGE(NAME)\
            );\
        }\
        template<class T = void,class ...Args>\
        int NAME(Args ...) const noexcept{\
            static_assert(\
                std::is_same_v<T,_Btk_shadow_t>,\
                BTK_SHADOWED_MESSAGE(NAME)\
            );\
        }
#else
    #define BTK_SHADOWED_MESSAGE(NAME)
    #define BTK_SHADOW_METHOD(NAME)
#endif
//Macro to avoid compile still develop code in msvc
//Bacause msvc will export all the symbols
#if defined(_MSC_VER) && !defined(BTK_VSCODE_SUPPRESS)
    #define BTK_STILL_DEV 0
#else
    #define BTK_STILL_DEV 1
#endif
//Debugs macro for header files
#ifdef BTK_HEADER_DEBUG_HINT
    #define BTK_H_INFO(...) _BtkL_Info(__VA_ARGS__)
    #define BTK_H_WARN(...) _BtkL_Warn(__VA_ARGS__)
    #define BTK_H_ERROR(...) _BtkL_Error(__VA_ARGS__)
#else
    #define BTK_H_INFO(...)
    #define BTK_H_WARN(...)
    #define BTK_H_ERROR(...)
#endif

//Debugs function
BTK_CDECLS_BEGIN

BTKAPI void _BtkL_Info(const char *fmt, ...);
BTKAPI void _BtkL_Warn(const char *fmt, ...);
BTKAPI void _BtkL_Error(const char *fmt, ...);

BTK_CDECLS_END

#if BTK_CXX
namespace Btk{
    //Some int defs in SDL2
    //Uint
    using Uint8 = uint8_t;
    using Uint16 = uint16_t;
    using Uint32 = uint32_t;
    using Uint64 = uint64_t;
    //Sint
    using Int8 = int8_t;
    using Int16 = int16_t;
    using Int32 = int32_t;
    using Int64 = int64_t;

    using Sint8 = int8_t;
    using Sint16 = int16_t;
    using Sint32 = int32_t;
    using Sint64 = int64_t;
    //end
    //Btk u16string
    class u32string_view;
    class u32string;
    class u16string_view;
    class u16string;
    class u8string_view;
    class u8string;
    /**
     * @brief Orientation of widget etc...
     * 
     */
    enum class Orientation:Uint8{
        Vertical = 0,
        Horizontal = 1,
        V = Vertical,
        H = Horizontal
    };
    inline constexpr auto Vertical = Orientation::Vertical; 
    inline constexpr auto Horizontal = Orientation::Horizontal; 

    //Useful min max climp template
    //Avoid to include algorithm in the headers
    #undef max
    #undef min

    template<class T>
    inline T max(const T &v1,const T &v2) BTK_NOEXCEPT_IF(v1 < v2){
        if(v1 > v2){
            return v1;
        }
        return v2;
    }
    template<class T>
    inline T min(const T &v1,const T &v2) BTK_NOEXCEPT_IF(v1 < v2){
        if(v1 < v2){
            return v1;
        }
        return v2;
    }
    /**
     * @brief Climp
     * 
     * @tparam T 
     * @param v 
     * @param min 
     * @param max 
     * @return T 
     */
    template<class T>
    inline T clamp(const T &v,const T &min,const T &max){
        if(v <= min){
            return min;
        }
        if(v >= max){
            return max;
        }
        return v;
    }
    /**
     * @brief Init Btk all subsystems
     * 
     * @return BTKAPI 
     */
    BTKAPI void Init();
    /**
     * @brief Quit Btk all subsystems,trigger AtExit callbacks
     * 
     * @return BTKAPI 
     */
    BTKAPI void Quit();
    /**
     * @brief Helper for initializing / quitting Btk
     * 
     */
    class Library{
        public:
            Library(){
                Init();
            }
            ~Library(){
                Quit();
            }
            Library(const Library &) = delete;
    };
}
#endif

#endif // _BTK_DEFS_HPP_
