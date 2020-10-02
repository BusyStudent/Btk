#include <SDL2/SDL.h>
#include <Btk/exception.hpp>
namespace Btk{
    SDLError::SDLError(const char *err):std::runtime_error(err){}
    SDLError::SDLError(const SDLError &err):std::runtime_error(err){}
    SDLError::~SDLError(){}
    //throwError
    [[noreturn]] void throwSDLError(){
        throw SDLError(SDL_GetError());
    }
    [[noreturn]] void throwSDLError(const char *err){
        throw SDLError(err);
    }
}