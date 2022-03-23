#if !defined(_BTK_PLATFORM_CONSOLE_COLOR_HPP_)
#define _BTK_PLATFORM_CONSOLE_COLOR_HPP_
#include <cstdio>
#include <string>
#include <string_view>
/**
 * @brief Some useful ansi console macro
 * 
 */

#define BTK_ANSI_COLOR_RED     "\x1b[31m"
#define BTK_ANSI_COLOR_GREEN   "\x1b[32m"
#define BTK_ANSI_COLOR_YELLOW  "\x1b[33m"
#define BTK_ANSI_COLOR_BLUE    "\x1b[34m"
#define BTK_ANSI_COLOR_MAGENTA "\x1b[35m"
#define BTK_ANSI_COLOR_CYAN    "\x1b[36m"
#define BTK_ANSI_COLOR_RESET   "\x1b[0m"

//Helper for using ansi color

namespace Btk::Console{
    struct _StdStream{
        FILE *out;
    };
    struct _Color:public std::string_view{
        using std::string_view::basic_string_view;
    };

    inline constexpr _Color Red = _Color(BTK_ANSI_COLOR_RED);
    inline constexpr _Color Green = _Color(BTK_ANSI_COLOR_GREEN);
    inline constexpr _Color Yellow = _Color(BTK_ANSI_COLOR_YELLOW);
    inline constexpr _Color Blue = _Color(BTK_ANSI_COLOR_BLUE);
    inline constexpr _Color Magenta = _Color(BTK_ANSI_COLOR_MAGENTA);
    inline constexpr _Color Cyan = _Color(BTK_ANSI_COLOR_CYAN);
    inline constexpr _Color Reset = _Color(BTK_ANSI_COLOR_RESET);


    inline _StdStream Stdin(){
        return {stdin};
    }
    inline _StdStream Stdout(){
        return {stdout};
    }
    inline _StdStream Stderr(){
        return {stderr};
    }

    inline _StdStream operator <<(_StdStream stream,const _Color &color){
        fputs(color.data(),stream.out);
        return stream;
    }
    inline _StdStream operator <<(_StdStream stream,const char *s){
        fputs(s,stream.out);
        return stream;
    }
    inline _StdStream operator <<(_StdStream stream,const std::string &s){
        fputs(s.c_str(),stream.out);
        return stream;
    }
    template<
        class T,
        typename _Cond = std::enable_if_t<
            std::is_arithmetic_v<T>
        >    
    >
    inline _StdStream operator <<(_StdStream stream,const T &t){
        fputs(std::to_string(t).c_str(),stream.out);
        return stream;
    }
}

#endif // _BTK_PLATFORM_CONSOLE_COLOR_HPP_
