#if !defined(_BTK_EXCEPTION_HPP_)
#define _BTK_EXCEPTION_HPP_
#include <stdexcept>
namespace Btk{
    //SDLError
    typedef std::runtime_error RuntimeError;
    class SDLError:public RuntimeError{
        public:
            SDLError(const char *err);
            SDLError(const SDLError &);
            ~SDLError();
    };
    class BadFunctionCall:public RuntimeError{
        public:
            BadFunctionCall();
            BadFunctionCall(const BadFunctionCall &);
            ~BadFunctionCall();
    };
    class RendererError:public RuntimeError{
        public:
            RendererError(const char *msg);
            RendererError(const RendererError &) = default;
            ~RendererError();
    };
    [[noreturn]] void throwRuntimeError(const char *);
    [[noreturn]] void throwSDLError(const char *);
    [[noreturn]] void throwSDLError();
    //throw BadFunctionCall
    [[noreturn]] void throwBadFunctionCall();

    [[noreturn]] void throwRendererError(const char *msg);
    [[noreturn]] void throwRendererError();
};


#endif // _BTK_EXCEPTION_HPP_
