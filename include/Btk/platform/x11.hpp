#if !defined(_BTK_PLATFORM_X11_HPP_)
#define _BTK_PLATFORM_X11_HPP_
#include <SDL2/SDL_events.h>
#include "../defs.hpp"
#include "../exception.hpp"
#include <string>
namespace Btk{
    namespace X11{
        BTKAPI void Init();
        BTKAPI void Quit();
        BTKAPI void HandleSysMsg(const SDL_SysWMmsg &);
        BTKAPI void MessageBox(std::string_view title,std::string_view msg);
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
