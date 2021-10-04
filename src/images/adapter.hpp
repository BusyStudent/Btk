#if !defined(_BTK_INTERNAL_IMAGE_ADAPTER_HPP_)
#define _BTK_INTERNAL_IMAGE_ADAPTER_HPP_

#include "../build.hpp"
#include <Btk/impl/loadso.hpp>
#include <Btk/impl/codec.hpp>
#include <Btk/impl/scope.hpp>
#include <SDL2/SDL_rwops.h>


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

#ifdef BTK_HAS_GIF
    BTKHIDDEN void RegisterGIF();
#else
    inline void RegisterGIF(){};
#endif
    //Helper class for save current pos status
    struct _RWopsStatusSaver{
        _RWopsStatusSaver(SDL_RWops *r){
            rwops = r;
            position = SDL_RWtell(r);
        }
        _RWopsStatusSaver(const _RWopsStatusSaver &) = delete;
        ~_RWopsStatusSaver(){
            SDL_RWseek(rwops,position,RW_SEEK_SET);
        }
        SDL_RWops *rwops;
        Sint64 position;
    };
}


#define BTK_LOCKSURFACE(SURF) \
    if(SDL_MUSTLOCK(SURF)){\
        SDL_LockSurface(SURF);\
    }
#define BTK_UNLOCKSURFACE(SURF) \
    if(SDL_MUSTLOCK(SURF)){\
        SDL_UnlockSurface(SURF);\
    }

#ifdef __COUNTER__
    #define BTK_RW_SAVE_STATUS(RW) \
        Btk::_RWopsStatusSaver __saver_##__COUNTER__(RW);
#else
    #define BTK_RW_SAVE_STATUS(RW) \
        Btk::_RWopsStatusSaver __saver(RW);
#endif

//Builtin Check
namespace Btk{
    //Type check from SDL image
    inline
    bool BultinIsGIF(SDL_RWops *rwops){
        BTK_RW_SAVE_STATUS(rwops);
        char magic[6];
        if(SDL_RWread(rwops,magic,sizeof(magic),1) != 1){
            return false;
        }
        return (SDL_strncmp(magic,"GIF",3) == 0);
    }
    inline
    bool BultinIsWebP(SDL_RWops *rwops){
        BTK_RW_SAVE_STATUS(rwops);
        char magic[20];
        if(SDL_RWread(rwops,magic,sizeof(magic),1) != 1){
            return false;
        }
        if(  magic[ 0] == 'R' &&
             magic[ 1] == 'I' &&
             magic[ 2] == 'F' &&
             magic[ 3] == 'F' &&
             magic[ 8] == 'W' &&
             magic[ 9] == 'E' &&
             magic[10] == 'B' &&
             magic[11] == 'P' &&
             magic[12] == 'V' &&
             magic[13] == 'P' &&
             magic[14] == '8' &&
            (magic[15] == ' ' || magic[15] == 'X' || magic[15] == 'L')
            ){
                return true;
        }
        return false;
    }
}

#endif // _BTK_INTERNAL_IMAGE_ADAPTER_HPP_
