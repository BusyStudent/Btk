#if !defined(_BTK_EXCEPTION_HPP_)
#define _BTK_EXCEPTION_HPP_
#include <stdexcept>
#include <exception>
#include <thread>
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
    
    enum class ThreadType{
        AsyncWorker,// The AsyncWorker
        Renderer,//< The Rendering thread
        Unknown,//< Unknown thread
    };
    struct ExceptionData{
        std::exception_ptr ptr;//<The exception
        std::thread::id whence;//<The thread id
        ThreadType thread_type;//<The thread type
    };
    //typedef bool (*ExceptionHandler)(ExceptionData);
    
    [[noreturn]] void BTKAPI throwRuntimeError(const char *);
    [[noreturn]] void BTKAPI throwSDLError(const char *);
    [[noreturn]] void BTKAPI throwSDLError();
    //throw BadFunctionCall
    [[noreturn]] void BTKAPI throwBadFunctionCall();

    [[noreturn]] void BTKAPI throwRendererError(const char *msg);
    [[noreturn]] void BTKAPI throwRendererError();
};


#endif // _BTK_EXCEPTION_HPP_
