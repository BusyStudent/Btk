#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/exception.hpp>
#include <Btk/render.hpp>
namespace Btk{
    SDLError::SDLError(const char *err):std::runtime_error(err){}
    SDLError::SDLError(const SDLError &err):std::runtime_error(err){}
    SDLError::~SDLError(){}
    //BadFunctionCall
    BadFunctionCall::BadFunctionCall():
        std::runtime_error("call an empty function"){}
    BadFunctionCall::BadFunctionCall(const BadFunctionCall &err):
        std::runtime_error(err){}
    BadFunctionCall::~BadFunctionCall(){}

    RendererError::RendererError(const char *msg):
        std::runtime_error(msg){}
    RendererError::~RendererError(){}
    //throwError
    [[noreturn]] void throwSDLError(){
        throw SDLError(SDL_GetError());
    }
    [[noreturn]] void throwSDLError(const char *err){
        throw SDLError(err);
    }
    [[noreturn]] void throwRuntimeError(const char *str){
        throw RuntimeError(str);
    }
    [[noreturn]] void throwBadFunctionCall(){
        throw BadFunctionCall();
    }
    [[noreturn]] void throwRendererError(const char *msg){
        throw RendererError(msg);
    }
    [[noreturn]] void throwRendererError(){
        throw RendererError(SDL_GetError());
    }
}