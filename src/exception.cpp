#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/exception.hpp>
#include <Btk/render.hpp>
namespace Btk{
    //RuntimeError
    RuntimeError::RuntimeError() = default;
    RuntimeError::~RuntimeError() = default;
    const char *RuntimeError::what() const noexcept{
        return _message.c_str();
    }

    SDLError::SDLError(u8string_view err):RuntimeError(err){}
    SDLError::SDLError(const SDLError &err):RuntimeError(err){}
    SDLError::~SDLError() = default;
    //BadFunctionCall
    BadFunctionCall::BadFunctionCall():
        RuntimeError("call an empty function"){}
    BadFunctionCall::BadFunctionCall(const BadFunctionCall &err):
        RuntimeError(err){}
    BadFunctionCall::~BadFunctionCall() = default;

    RendererError::RendererError(u8string_view msg):
        RuntimeError(msg){}
    RendererError::~RendererError() = default;
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