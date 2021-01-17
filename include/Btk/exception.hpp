#if !defined(_BTK_EXCEPTION_HPP_)
#define _BTK_EXCEPTION_HPP_
#include <stdexcept>
#include "defs.hpp"
namespace Btk{
    //SDLError
    typedef std::runtime_error RuntimeError;
    class BTKAPI SDLError:public RuntimeError{
        public:
            SDLError(const char *err);
            SDLError(const SDLError &);
            ~SDLError();
    };
    class BTKAPI BadFunctionCall:public RuntimeError{
        public:
            BadFunctionCall();
            BadFunctionCall(const BadFunctionCall &);
            ~BadFunctionCall();
    };
    class BTKAPI RendererError:public RuntimeError{
        public:
            RendererError(const char *msg);
            RendererError(const RendererError &) = default;
            ~RendererError();
    };
    [[noreturn]] void BTKAPI throwRuntimeError(const char *);
    [[noreturn]] void BTKAPI throwSDLError(const char *);
    [[noreturn]] void BTKAPI throwSDLError();
    //throw BadFunctionCall
    [[noreturn]] void BTKAPI throwBadFunctionCall();

    [[noreturn]] void BTKAPI throwRendererError(const char *msg);
    [[noreturn]] void BTKAPI throwRendererError();
};


#endif // _BTK_EXCEPTION_HPP_
