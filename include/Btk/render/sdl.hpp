#if !defined(_BTK_RENDERER_SDL_HPP_)
#define _BTK_RENDERER_SDL_HPP_
#include <SDL2/SDL_render.h>
#include "./render.hpp"
namespace Btk{
    /**
     * @brief SDL_Render Impl
     * 
     */
    class SDLRenderer:public Renderer{
        public:

        private:
            SDL_Renderer *render;
    };
};


#endif // _BTK_RENDERER_SDL_HPP_
