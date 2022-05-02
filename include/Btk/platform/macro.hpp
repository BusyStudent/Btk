#if !defined(_BTK_PLATFORM_MACRO_HPP_)
#define _BTK_PLATFORM_MACRO_HPP_

//CXX --begin
#ifdef __cplusplus
    #define BTK_CXX17 (__cplusplus >= 201703L)
    #define BTK_CXX20 (__cplusplus > 201703L)
    //C++ 20 __cplusplus is bigger than C++17
    //Because many compiler has different value
    #define BTK_CXX 1
    static_assert(BTK_CXX17,"We need a C++17 to compile btk");
#else
    #define BTK_CXX 0
#endif
//CXX --end


//Complier --begin
#define BTK_MINGW 0
#define BTK_CLANG 0
#define BTK_MSVC 0
#define BTK_GCC 0

#ifdef __GNUC__
    #undef  BTK_GCC 
    #define BTK_GCC 1
#endif

#ifdef _MSC_VER
    #undef  BTK_MSVC 
    #define BTK_MSVC 1
#endif

#ifdef __clang__
    #undef  BTK_CLANG
    #define BTK_CLANG 1
#endif

#ifdef _WIN32
    #undef  BTK_MINGW
    #define BTK_MINGW (1 && BTK_GCC)
#endif
//Complier --end



//Platform --begin
#define BTK_ANDROID 0
#define BTK_WIN32   0
#define BTK_LINUX   0
#define BTK_X11     0
#define BTK_IOS     0

#ifdef __ANDROID__
    #undef  BTK_ANDROID
    #define BTK_ANDROID 1
#endif

#ifdef __linux
    #undef  BTK_LINUX
    #define BTK_LINUX 1
#endif

#ifdef __gnu_linux__
    #undef  BTK_X11
    #define BTK_X11 1
#endif

#ifdef _WIN32
    #undef  BTK_WIN32
    #define BTK_WIN32 1
#endif

#ifdef __IPHONES__
    #undef  BTK_IOS
    #define BTK_IOS 1
#endif

#define BTK_MOBILE (BTK_IOS || BTK_ANDROID)
#define BTK_DESKTOP (BTK_WIN32 || BTK_X11)
//Platform --end

//Export symbols --begin
#if   BTK_MINGW
    #define BTKEXPORT __attribute__((dllexport))
    #define BTKIMPORT __attribute__((dllimport))
    #define BTKCDEL   __attribute__((cdel))
    #define BTKFAST   __attribute__((fastcall))
#elif BTK_MSVC
    #define BTKEXPORT __declspec(dllexport)
    #define BTKIMPORT __declspec(dllimport)
    #define BTKCDEL   __cdel
    #define BTKFAST   __fastcall

    #undef not
    #undef and
    #undef or

    #define not !
    #define and &&
    #define or ||
#elif BTK_GCC
    #define BTKEXPORT __attribute__((visibility("default")))  
    #define BTKIMPORT 
    #define BTKCDEL
    #define BTKFAST   __attribute__((fastcall))
#else
    //Ignored
    #define BTKCDEL
    #define BTKEXPORT 
    #define BTKIMPORT
    #define BTKFAST
#endif
//Export symbols --end

//Complier attributes --begin
#if   BTK_MSVC
    #define BTKWEAK 
    #define BTKHIDDEN  
    #define BTKINLINE __forceinline
    
    #define BTK_ATTRIBUTE(...) __declspec(__VA_ARGS__)
    #define BTK_NODISCARD(MSG) __declspec((nodiscard(MSG)))
    #define BTK_FUNCTION __FUNCSIG__
#elif BTK_GCC
    #define BTKWEAK   __attribute__((weak))
    #define BTKHIDDEN __attribute__((visibility("hidden")))
    #define BTKINLINE __attribute__((__always_inline__))

    #define BTK_ATTRIBUTE(...) __attribute__((__VA_ARGS__))
    #define BTK_NODISCARD(MSG) __attribute__((nodiscard(MSG)))
    #define BTK_FUNCTION __PRETTY_FUNCTION__
#else
    #define BTKWEAK 
    #define BTKHIDDEN
    #define BTKINLINE 

    #define BTK_ATTRIBUTE(...)
    #define BTK_NODISCARD(MSG)
    #define BTK_FUNCTION __FUNCTION__
#endif
//Complier attributes --end

//Bultin
#ifdef __has_builtin
    #define BTK_HAS_BULTIN(X) __has_builtin
#else
    #define BTK_HAS_BULTIN(X) 0
#endif

//Architecture
#ifdef __ARM__
    #undef  BTKFAST
    #define BTKFAST
#endif

#endif // _BTK_PLATFORM_MACRO_HPP_
