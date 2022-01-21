#if !defined(_BTK_PLATFORM_X11_HPP_)
#define _BTK_PLATFORM_X11_HPP_
#include <SDL2/SDL_events.h>
#include "../defs.hpp"
#include "../string.hpp"
#include "../exception.hpp"
#include <cstdio>
#include <string>
struct SDL_Window;
namespace Btk{
    namespace X11{
        BTKAPI void Init();
        BTKAPI void Quit();
        BTKAPI void HandleSysMsg(const SDL_SysWMmsg &);
        BTKAPI bool MessageBox(u8string_view title,u8string_view msg,int flag = 0);

        SDL_Window *CreateTsWindow();
    };
    //X11 Error
    class BTKAPI XError:public RuntimeError{
        public:
            /**
             * @brief Construct a new XError object
             * 
             * @param x_display The Display
             * @param p_xevent The pointer to XErrorEvent
             */
            XError(const void *p_xevent);
            XError(const XError &) = default;
            ~XError();
        private:
            void *_x_display;
    };
};


#endif // _BTK_PLATFORM_X11_HPP_
