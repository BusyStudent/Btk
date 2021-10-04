#include "../build.hpp"

#include <Btk/impl/loadso.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/rwops.hpp>
#include <csetjmp>
#include <png.h>

#define BTK_HAS_PNG
#include "adapter.hpp"

//PNG LIBRARY
#include <png.h>

#define PNG_TRY(CTXT) if(::setjmp(png_jmpbuf(CTXT)) == 0)
#define PNG_CATCH else


namespace{
    #include "loader/png.hpp"
    //Make deleter
    // struct png_deleter{
    //     void operator ()(png_structp){
    //         png_destroy_read_struct()
    //     }
    // };
    // using png_struct_ptr = std::unique_ptr<png_structp,png_deleter>;

    bool check_is_png(SDL_RWops *rwops){
        Uint8 magic[8];
        //Save cur
        auto cur = SDL_RWtell(rwops);
        //Read magic
        SDL_RWread(rwops,magic,sizeof(magic),1);
        //Reset to the position
        SDL_RWseek(rwops,cur,RW_SEEK_SET);
        //Check
        return png_sig_cmp(magic,0,sizeof(magic)) == 0;
    }
    void png_read_data(png_structp reader,png_bytep buf,size_t n){
        void *rwops = png_get_io_ptr(reader);
        if(SDL_RWread(static_cast<SDL_RWops*>(rwops),buf,1,n) != n){
            //Read Error
            png_error(reader,SDL_GetError());
        }
    }
    SDL_Surface *png_load(SDL_RWops *rwops){
        if(not check_is_png(rwops)){
            SDL_SetError("Invalid png");
            return nullptr;
        }
        png_struct *reader = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            nullptr,
            nullptr,
            nullptr
        );
        if(reader == nullptr){
            return nullptr;
        }
        png_set_read_fn(reader,rwops,png_read_data);
        png_destroy_read_struct(&reader,nullptr,nullptr);
        return nullptr;
        
    }
}

namespace Btk{
    struct BTKHIDDEN PngDecoder:public ImageDecoder{
        PngDecoder() = default;
    };
}

namespace Btk{
    void RegisterPNG(){
        ImageAdapter adapter;
        adapter.name = "png";
        adapter.vendor = "libpng";
        adapter.fn_is = check_is_png;
        adapter.fn_load = png_load;
        BTK_PNG_LOAD();
        RegisterImageAdapter(adapter);
    }
}