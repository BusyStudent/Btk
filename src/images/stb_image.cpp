
#include "../build.hpp"
#include "./adapter.hpp"

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_rwops.h>
#include <Btk/impl/codec.hpp>
#include <Btk/exception.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#define STBI_NO_STDIO

#define STBI_ASSERT(X) BTK_ASSERT(X)
#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free

extern "C"{
    #include "../libs/stb_image.h"
}

//Make check functions
#define BTK_STBI_IS(TYPE) [](SDL_RWops *rwops) -> bool{\
    stbi_io_cb callback(rwops);\
    stbi__context s;\
    stbi__start_callbacks(&s,(stbi_io_callbacks *)&callback,&callback);\
    return stbi__##TYPE##_test(&s);\
}
#define BTK_STBI_WRAPPER(TYPE) {\
        ImageAdapter adapter;\
        adapter.vendor = "stbi";\
        adapter.name = #TYPE;\
        adapter.fn_load = stbi_load;\
        adapter.fn_is = BTK_STBI_IS(TYPE);\
        RegisterImageAdapter(adapter);\
}

namespace{
    //TODO ADD Error check
    //Callback wrapper for stb_callback
    struct BTKHIDDEN stbi_io_cb:public stbi_io_callbacks{
        stbi_io_cb(SDL_RWops *rwops){
            read = [](void *user,char *data,int size){
                return static_cast<stbi_io_cb*>(user)->_read(data,size);
            };
            eof = [](void *user){
                return static_cast<stbi_io_cb*>(user)->_eof();
            };
            skip = [](void *user,int n){
                return static_cast<stbi_io_cb*>(user)->_skip(n);
            };
            fptr = rwops;
            //Set current position
            cur = SDL_RWtell(fptr);
            //Seek to end
            SDL_RWseek(fptr,0,RW_SEEK_END);
            //Get end
            end = SDL_RWtell(fptr);
            //Reset
            SDL_RWseek(fptr,cur,RW_SEEK_SET);
        }
        int _read(char *data,int size){
            return SDL_RWread(fptr,data,sizeof(char),size);
        }
        int _eof(){
            //Is eof
            return SDL_RWtell(fptr) == end;
        }
        void _skip(int n){
            //Seek at cur
            SDL_RWseek(fptr,n,RW_SEEK_CUR);
        }
        SDL_RWops *fptr;
        Sint64 cur;//Current pos
        Sint64 end;//The end position
    };

    SDL_Surface *stbi_load(SDL_RWops *rwops){
        stbi_io_cb callback(rwops);
        int w,h;
        int comp;
        
        stbi_uc *data = stbi_load_from_callbacks(&callback,&callback,&w,&h,&comp,STBI_rgb_alpha);
        if(data == nullptr){
            SDL_SetError("%s",stbi_failure_reason());
            return nullptr;
        }
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
            data,
            w,h,
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32),
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32) * w,
            SDL_PIXELFORMAT_RGBA32
        );
        stbi_image_free(data);

        if(surf == nullptr){
            Btk::throwSDLError();
        }

        return surf;
    }

}

namespace Btk{
    void RegisterSTBII(){
        stbi_set_unpremultiply_on_load(1);
        stbi_convert_iphone_png_to_rgb(1);
        
        BTK_STBI_WRAPPER(jpeg);        
        BTK_STBI_WRAPPER(png);        
        BTK_STBI_WRAPPER(tga);        
        BTK_STBI_WRAPPER(hdr);        
        BTK_STBI_WRAPPER(pic);        
        BTK_STBI_WRAPPER(bmp);
        BTK_STBI_WRAPPER(psd);
        BTK_STBI_WRAPPER(pnm);
        //TODO ADD gif decoder
    }
}