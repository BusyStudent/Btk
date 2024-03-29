#if !defined(_BTK_INTERNAL_IMAGE_ADAPTER_HPP_)
#define _BTK_INTERNAL_IMAGE_ADAPTER_HPP_

#include "../build.hpp"
#include <Btk/detail/loadso.hpp>
#include <Btk/detail/codec.hpp>
#include <Btk/detail/scope.hpp>
#include <SDL2/SDL_rwops.h>


namespace Btk{

#ifdef BTK_HAVE_LIBPNG
    BTKHIDDEN void RegisterPNG();
#else
    inline void RegisterPNG(){}
#endif

#ifdef BTK_HAVE_SDL_IMAGE
    BTKHIDDEN void RegisterSDLImage();
#else
    inline void RegisterSDLImage(){}
#endif

#ifdef BTK_HAVE_STB_IMAGE
    BTKHIDDEN void RegisterSTBII();
#else
    inline void RegisterSTBII(){}
#endif

#ifdef BTK_HAVE_LIBWEBP
    BTKHIDDEN void RegisterWEBP();
#else
    inline void RegisterWEBP(){}
#endif

#ifdef BTK_HAVE_WINCODEC
    BTKHIDDEN void RegisterWIC();
#else
    inline void RegisterWIC(){}
#endif

#ifdef BTK_HAVE_GIFLIB
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

#define BTK_RW_SAVE_STATUS(RW) \
    Btk::_RWopsStatusSaver BTK_UNIQUE_NAME(__saver) (RW);

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
    inline
    bool BultinIsXPM(SDL_RWops *rwops){
        BTK_RW_SAVE_STATUS(rwops);
        constexpr auto size = sizeof("/* XPM */") - sizeof(char);
        constexpr auto req_magic = "/* XPM */";

        char magic[size];
        if(SDL_RWread(rwops,magic,sizeof(magic),1) != 1){
            return false;
        }
        return SDL_strncasecmp(magic,req_magic,size) == 0;
    }
    inline
    bool BultinIsBMP(SDL_RWops *rwops){
        //Check magic is bm
        char magic[2] = {0};
        //Save cur
        auto cur = SDL_RWtell(rwops);
        //Read magic
        SDL_RWread(rwops,magic,sizeof(magic),1);
        //Reset to the position
        SDL_RWseek(rwops,cur,RW_SEEK_SET);
        //Check magic
        return magic[0] == 'B' and magic[1] == 'M';
    }
    inline
    Sint64 RWtellsize(SDL_RWops *rwops){
        Sint64 cur = SDL_RWtell(rwops);
        SDL_RWseek(rwops,0,RW_SEEK_END);
        Sint64 end = SDL_RWtell(rwops);
        SDL_RWseek(rwops,cur,RW_SEEK_SET);
        return end - cur;
    }
}

inline
size_t SDL_WriteString(SDL_RWops *rw,const char *str){
    return SDL_RWwrite(rw,str,sizeof(char),SDL_strlen(str));
}
inline
size_t SDL_RWpinrt(SDL_RWops *rw,const char *fmt,...){
    va_list varg;
    va_start(varg,fmt);
    auto t = Btk::u8vformat(fmt,varg);
    va_end(varg);
    return SDL_RWwrite(rw,t.c_str(),sizeof(char),t.size());
}

#endif // _BTK_INTERNAL_IMAGE_ADAPTER_HPP_
