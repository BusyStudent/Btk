#include "../build.hpp"

#include <Btk/gl/gl.hpp>
#include <SDL2/SDL.h>

namespace Btk{
namespace GL{
    void Init(){
        //Tell the sdl,We want to use OPENGLES
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
        //Load library
        #ifdef BTK_NEED_GLAD
        if(SDL_GL_LoadLibrary(nullptr) == -1){
            BTK_LOGINFO("Failed to Load GLES %s",SDL_GetError());
        }
        #endif
    }
    void Quit(){
        #ifdef BTK_NEED_GLAD
        SDL_GL_UnloadLibrary();
        #endif
    }
}
}