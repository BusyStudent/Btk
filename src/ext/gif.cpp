#include "../build.hpp"

#include <SDL2/SDL_rwops.h>
#include <Btk/pixels.hpp>
#include <gif_lib.h>

namespace{
    int input_wrapper(GifFileType *fp,GifByteType *data,int len){
        return SDL_RWread(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }
    int output_wrapper(GifFileType *fp,const GifByteType *data,int len){
        return SDL_RWwrite(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }

    inline GifFileType *get_gif(void *ptr){
        return static_cast<GifFileType*>(ptr);
    }
}
namespace Btk{
    
}