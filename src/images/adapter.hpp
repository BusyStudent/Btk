#if !defined(_BTK_INTERNAL_IMAGE_ADAPTER_HPP_)
#define _BTK_INTERNAL_IMAGE_ADAPTER_HPP_

#include "../build.hpp"

namespace Btk{

#ifdef BTK_HAS_PNG
    BTKHIDDEN void RegisterPNG();
#else
    inline void RegisterPNG(){}
#endif

#ifdef BTK_HAS_SDLIMG
    BTKHIDDEN void RegisterSDLImage();
#else
    inline void RegisterSDLImage(){}
#endif

#ifdef BTK_HAS_STBII
    BTKHIDDEN void RegisterSTBII();
#else
    inline void RegisterSTBII(){}
#endif

#ifdef BTK_HAS_WEBP
    BTKHIDDEN void RegisterWEBP();
#else
    inline void RegisterWEBP(){}
#endif

#ifdef BTK_HAS_WIC
    BTKHIDDEN void RegisterWIC();
#else
    inline void RegisterWIC(){}
#endif

}


#define BTK_LOCKSURFACE(SURF) \
    if(SDL_MUSTLOCK(SURF)){\
        SDL_LockSurface(SURF);\
    }
#define BTK_UNLOCKSURFACE(SURF) \
    if(SDL_MUSTLOCK(SURF)){\
        SDL_UnlockSurface(SURF);\
    }

#endif // _BTK_INTERNAL_IMAGE_ADAPTER_HPP_
