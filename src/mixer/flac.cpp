#include "../build.hpp"

#include <Btk/impl/mixer.hpp>
#include <FLAC/all.h>

namespace{
    static size_t flac_read(void *ptr,size_t size,size_t n,FLAC__IOHandle handle){
        return SDL_RWread(static_cast<SDL_RWops*>(handle),ptr,size,n);
    }
    static size_t flac_write(const void *ptr,size_t size,size_t n,FLAC__IOHandle handle){
        return SDL_RWwrite(static_cast<SDL_RWops*>(handle),ptr,size,n);
    }
}