#if !defined(_BTK_GL_HPP_)
#define _BTK_GL_HPP_

#ifndef __linux
    #define BTK_NEED_GLAD
#endif

#ifdef BTK_NEED_GLAD
    #include "glad.h"
#else
    #include <SDL2/SDL_opengles2.h>
#endif
namespace Btk{
namespace GL{
    void Init();
}
}

#endif // _BTK_GL_HPP_
