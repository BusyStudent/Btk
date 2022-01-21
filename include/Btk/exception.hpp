#if !defined(_BTK_EXCEPTION_HPP_)
#define _BTK_EXCEPTION_HPP_
// #include <stdexcept>
#include <exception>
#include <cerrno>
// #include <thread>
#include "defs.hpp"
#include "string.hpp"
namespace Btk{
    //SDLError
    /**
     * @brief Btk's message
     * 
     */
    class BTKAPI RuntimeError:public std::exception{
        public:
            RuntimeError() = default;
            RuntimeError(const RuntimeError &) = default;
            RuntimeError(RuntimeError &&) = default;
            RuntimeError(u8string_view message):_message(message){}
            ~RuntimeError();

            const char *what() const noexcept;

            void set_message(u8string_view msg){
                _message = msg;
            }
            u8string &message(){
                return _message;
            }
            const u8string &message() const{
                return _message;
            }
        private:
            u8string _message;
    };
    class BTKAPI SDLError:public RuntimeError{
        public:
            SDLError(u8string_view err):RuntimeError(err){};
            SDLError(const SDLError &) = default;
            ~SDLError();
    };
    class BTKAPI BadFunctionCall:public RuntimeError{
        public:
            BadFunctionCall();
            BadFunctionCall(const BadFunctionCall &) = default;
            ~BadFunctionCall();
    };
    class BTKAPI RendererError:public RuntimeError{
        public:
            RendererError(u8string_view msg):RuntimeError(msg){};
            RendererError(const RendererError &) = default;
            ~RendererError();
    };
    /**
     * @brief Error from errno
     * 
     */
    class BTKAPI CRuntimeError:public RuntimeError{
        public:
            CRuntimeError(int _errno){
                set_errcode(_errno);
            }
            CRuntimeError(const CRuntimeError &) = default;
            ~CRuntimeError();

            void set_errcode(int _errno);
            int  errcode() const noexcept{
                return _errno;
            }
        private:
            int _errno = {};
    };
    class BTKAPI IndexError:public RuntimeError{
        public:

        private:
            union{
                size_t _ulidx;
                long   _lidx;
            };
    };
    // enum class ThreadType{
    //     AsyncWorker,// The AsyncWorker
    //     Renderer,//< The Rendering thread
    //     Unknown,//< Unknown thread
    // };
    // struct ExceptionData{
    //     std::exception_ptr ptr;//<The exception
    //     std::thread::id whence;//<The thread id
    //     ThreadType thread_type;//<The thread type
    // };
    //typedef bool (*ExceptionHandler)(ExceptionData);
    [[noreturn]] void BTKAPI throwCRuntimeError(int errcode = errno);
    [[noreturn]] void BTKAPI throwRuntimeError(u8string_view msg);
    [[noreturn]] void BTKAPI throwSDLError(u8string_view msg);
    [[noreturn]] void BTKAPI throwSDLError();
    //throw BadFunctionCall
    [[noreturn]] void BTKAPI throwBadFunctionCall();

    [[noreturn]] void BTKAPI throwRendererError(u8string_view msg);
    [[noreturn]] void BTKAPI throwRendererError();
};


#endif // _BTK_EXCEPTION_HPP_
