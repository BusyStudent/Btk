#include "../build.hpp"

#include <Btk/gl/gl.hpp>
#include <SDL2/SDL.h>

namespace Btk{
namespace GL{
    void Init(){
        //Tell the sdl,We want to use OPENGLES
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    }
    void Quit(){
        
    }
}
}