#if !defined(_BTK_EXCEPTION_HPP_)
#define _BTK_EXCEPTION_HPP_
#include <stdexcept>
namespace Btk{
    //SDLError
    typedef std::runtime_error RuntimeError;
    class SDLError:public std::runtime_error{
        public:
            SDLError(const char *err);
            SDLError(const SDLError &);
            ~SDLError();
    };
    [[noreturn]] void throwRuntimeError(const char *);
    [[noreturn]] void throwSDLError(const char *);
    [[noreturn]] void throwSDLError();
};


#endif // _BTK_EXCEPTION_HPP_
