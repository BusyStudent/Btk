#if !defined(_BTK_PLATFORM_X11_HPP_)
#define _BTK_PLATFORM_X11_HPP_
#include <SDL2/SDL_events.h>
#include "../defs.hpp"
#include "../string.hpp"
#include "../exception.hpp"
#include <cstdio>
#include <string>
namespace Btk{
    namespace X11{
        BTKAPI void Init();
        BTKAPI void Quit();
        BTKAPI void HandleSysMsg(const SDL_SysWMmsg &);
        BTKAPI bool MessageBox(u8string_view title,u8string_view msg,int flag = 0);
        /**
         * @brief Execute 
         * 
         * @param argc The number of the args
         * @param ... The args string
         * @return int 
         */
        BTKAPI int   VExecute(size_t argc,...);
        BTKAPI FILE* VPopen(size_t argc,...);

        inline const char *_GetCString(const char * s){
            return s;
        }
        inline const char *_GetCString(const std::string &s){
            return s.c_str();
        }
        inline const char *_GetCString(const u8string &s){
            return s.c_str();
        }
        inline const char *_GetCString(std::string_view s){
            return s.data();
        }
        inline const char *_GetCString(u8string_view s){
            return s.data();
        }

        template<class ...Args>
        int Execute(Args &&...args){
            return VExecute(sizeof...(args),_GetCString(std::forward<Args>(args))...);
        }
        template<class ...Args>
        FILE *Popen(Args &&...args){
            return Popen(sizeof...(args),_GetCString(std::forward<Args>(args))...);
        }
    };
    //X11 Error
    class XError:public RuntimeError{
        public:
            XError() = default;
            XError(const XError &) = default;
            ~XError();
            const char *what() const noexcept;
    };
};


#endif // _BTK_PLATFORM_X11_HPP_
