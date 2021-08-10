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
//Dymaic Library
#ifdef BTK_PNG_DYMAIC
    #define BTK_PNG_LIBRARY  BTK_DYMAIC_LIBRARY
    #define BTK_PNG_FUNCTION BTK_DYMAIC_FUNCTION
#else
    #define BTK_PNG_LIBRARY BTK_STATIC_LIBRARY
    #define BTK_PNG_FUNCTION BTK_STATIC_FUNCTION
#endif

#ifdef _WIN32
    #define PNG_LIBNAME "libpng16.dll"
#else
    #define PNG_LIBNAME "libpng16.so"
#endif


#define PNG_TRY(CTXT) if(::setjmp(png_jmpbuf(CTXT)) == 0)
#define PNG_CATCH else


namespace{    
    struct BTKHIDDEN PngLibrary{
        BTK_PNG_LIBRARY(PNG_LIBNAME);

        BTK_PNG_FUNCTION(png_create_info_struct);
        BTK_PNG_FUNCTION(png_create_read_struct);
        BTK_PNG_FUNCTION(png_destroy_info_struct);
        BTK_PNG_FUNCTION(png_destroy_read_struct);

        BTK_PNG_FUNCTION(png_set_longjmp_fn);
        BTK_PNG_FUNCTION(png_set_read_fn);
        BTK_PNG_FUNCTION(png_get_IHDR);
        BTK_PNG_FUNCTION(png_read_info);
        BTK_PNG_FUNCTION(png_get_io_ptr);
        BTK_PNG_FUNCTION(png_set_sig_bytes);
        BTK_PNG_FUNCTION(png_sig_cmp);
        BTK_PNG_FUNCTION(png_error);

    };
    #ifndef BTK_PNG_DYMAIC
    //Create a static libaray
    BTK_MAKE_STLIB(PngLibrary,pnglib);
    #else
    BTK_MAKE_DYLIB(PngLibrary,pnglib);
    #endif

    //Make png 
    #define png_error pnglib->png_error
    #define png_sig_cmp pnglib->png_sig_cmp
    #define png_get_io_ptr pnglib->png_get_io_ptr
    #define png_set_read_fn pnglib->png_set_read_fn
    #define png_create_read_struct pnglib->png_create_read_struct
    #define png_destroy_read_struct pnglib->png_destroy_read_struct
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
        pnglib.load();
        RegisterImageAdapter(adapter);
    }
}