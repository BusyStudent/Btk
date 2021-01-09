#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/exception.hpp>
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
}