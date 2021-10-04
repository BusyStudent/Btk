#if !defined(_BTK_DEFS_HPP_)
#define _BTK_DEFS_HPP_

#ifdef __cplusplus
    #include <cstdint>
    #if __cplusplus < 201703L
    #error We need a C++17 to compile btk
    #endif
    //C++ 20 
    #if __cplusplus > 201703L
        //C++ 20 __cplusplus is bigger than C++17
        //Because many compiler has different value
        #define BTK_CXX20
        #define BTK_REQUIRE_CXX20 1
    #else
        #define BTK_REQUIRE_CXX20 0
    #endif
    //For C functions
    #define BTK_CEXTERN extern "C"
    #define BTK_CDECLS_BEGIN extern "C"{
    #define BTK_CDECLS_END }
#else
    #include <stdint.h>
    #define BTK_CEXTERN
    #define BTK_CDECLS_BEGIN
    #define BTK_CDECLS_END
#endif

#ifdef _WIN32
    #ifdef _MSC_VER
        #define BTKEXPORT __declspec(dllexport)
        #define BTKIMPORT __declspec(dllimport)
        #undef not
        #undef and
        #undef or

        #define not !
        #define and &&
        #define or ||
    #else
        #define BTKEXPORT __attribute__((dllexport))
        #define BTKIMPORT __attribute__((dllimport))
    #endif
    #define BTKCDEL __cdel
#elif defined(__GNUC__)
    #define BTKCDEL //Ignore CDEL call
    #define BTKEXPORT __attribute__((visibility("default")))  
    #define BTKIMPORT 
#else
    //ignore this
    #define BTKCDEL
    #define BTKEXPORT 
    #define BTKIMPORT 
#endif

//attributes
#if defined(__GNUC__)
    #define BTKWEAK   __attribute__((weak))
    #define BTKHIDDEN __attribute__((visibility("hidden")))
    #define BTKINLINE __attribute__((__always_inline__))

    #define BTK_NODISCARD(MSG) __attribute__((nodiscard(MSG)))
#elif defined(_MSC_VER)    
    #define BTKWEAK 
    #define BTKHIDDEN  
    #define BTKINLINE __forceinline
    
    #define BTK_NODISCARD(MSG) __declspec((nodiscard(MSG)))
#else
    #define BTKWEAK 
    #define BTKHIDDEN
    #define BTKINLINE 
#endif
//rename macro
#define BTK_PRIVATE(NAME) __BtkPriv_##NAME
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
    inline ENUM operator OP##=(ENUM a1,ENUM a2) noexcept{\
        return static_cast<ENUM>(\
            static_cast<BASE>(a1) OP static_cast<BASE>(a2)\
        );\
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


#ifdef _BTK_SOURCE
    //source file
    #define BTKAPI BTKEXPORT
#else
    //#define BTKAPI BTKIMPORT
    #define BTKAPI
#endif

//Platform checking
#if defined(_WIN32) || defined(__gnu_linux__)
    #define BTK_DESKTOP
#elif defined(__ANDROID__) || defined(__IPHONEOS__)
    #define BTK_MOBILE
#endif
//Macro to avoid compile still develop code in msvc
//Bacause msvc will export all the symbols
#if defined(_MSC_VER) && !defined(BTK_VSCODE_SUPPRESS)
    #define BTK_STILL_DEV 0
#else
    #define BTK_STILL_DEV 1
#endif

#ifdef __cplusplus
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
    class u16string_view;
    class u16string;
    class u8string_view;
    class u8string;
    //Generic Align
    
    enum class Orientation:unsigned int{
        Vertical = 0,
        Horizontal = 1,
        V = Vertical,
        H = Horizontal
    };
    //Useful min max climp template
    //Avoid to include algorithm in the headers
    template<class T>
    inline T max(const T &v1,const T &v2){
        if(v1 > v2){
            return v1;
        }
        return v2;
    }
    template<class T>
    inline T min(const T &v1,const T &v2){
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
}
#endif

#endif // _BTK_DEFS_HPP_
