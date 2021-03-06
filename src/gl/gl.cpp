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
        //Load library
        if(SDL_GL_LoadLibrary(nullptr) == -1){
            BTK_LOGINFO("Failed to Load GLES %s",SDL_GetError());
        }
        //Load it
        BtkLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress));

        BTK_LOGINFO("GLVersion %d.%d",GLVersion.major,GLVersion.minor);
    }
    void Quit(){
        SDL_GL_UnloadLibrary();
    }
}
}