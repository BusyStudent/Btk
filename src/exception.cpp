#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/exception.hpp>
#include <Btk/render.hpp>
namespace Btk{
    //RuntimeError
    RuntimeError::~RuntimeError() = default;
    const char *RuntimeError::what() const noexcept{
        return _message.c_str();
    }
    SDLError::~SDLError() = default;
    //BadFunctionCall
    BadFunctionCall::BadFunctionCall():
        RuntimeError("call an empty function"){}
    BadFunctionCall::~BadFunctionCall() = default;

    RendererError::~RendererError() = default;
    CRuntimeError::~CRuntimeError() = default;
    void CRuntimeError::set_errcode(int err){
        _errno = err;
        set_message(u8format("[errno %d] %s",_errno,std::strerror(_errno)));
    }
    //throwError
    [[noreturn]] void throwSDLError(){
        _Btk_Backtrace();
        throw SDLError(SDL_GetError());
    }
    [[noreturn]] void throwSDLError(u8string_view err){
        _Btk_Backtrace();
        throw SDLError(err);
    }
    [[noreturn]] void throwRuntimeError(u8string_view str){
        _Btk_Backtrace();
        throw RuntimeError(str);
    }
    [[noreturn]] void throwCRuntimeError(int e){
        _Btk_Backtrace();
        throw CRuntimeError(e);
    }
    [[noreturn]] void throwBadFunctionCall(){
        _Btk_Backtrace();
        throw BadFunctionCall();
    }
    [[noreturn]] void throwRendererError(u8string_view msg){
        _Btk_Backtrace();    
        throw RendererError(msg);
    }
    [[noreturn]] void throwRendererError(){
        _Btk_Backtrace();
        throw RendererError(SDL_GetError());
    }
}