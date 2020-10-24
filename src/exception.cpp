#include <SDL2/SDL.h>
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
    //EmptySignal
    EmptySignal::EmptySignal():
        std::runtime_error("emit an empty signal"){}
    EmptySignal::EmptySignal(const EmptySignal &err):
        std::runtime_error(err){}
    EmptySignal::~EmptySignal(){};
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
    [[noreturn]] void throwEmptySignal(){
        throw EmptySignal();
    }
}