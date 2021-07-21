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
    using Btk::ImageLibrary;
    
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

        bool is_png(SDL_RWops *rwops) const;
        SDL_Surface *load_png(SDL_RWops *rwops) const;
    };
    #ifndef BTK_PNG_DYMAIC
    //Create a static libaray
    BTK_MAKE_STLIB(PngLibrary,pnglib);
    #else
    BTK_MAKE_DYLIB(PngLibrary,pnglib);
    #endif
    static void read_callback(png_structp png,png_bytep buf,size_t size){
        void *rw = pnglib->png_get_io_ptr(png);
        size_t n = SDL_RWread(static_cast<SDL_RWops*>(rw),buf,1,size);
        //failure
        if(n == 0){
            pnglib->png_error(png,SDL_GetError());
        }
    }
    bool PngLibrary::is_png(SDL_RWops *rwops) const{
        constexpr int PNG_SIG_BYTE = 8;
        char buf[PNG_SIG_BYTE];

        SDL_RWseek(rwops,0,RW_SEEK_SET);

        if(SDL_RWread(rwops,buf,1,PNG_SIG_BYTE) != PNG_SIG_BYTE){
            //FAILED to read
            return false;
        }
        return png_sig_cmp(reinterpret_cast<png_const_bytep>(buf),0,PNG_SIG_BYTE)== 0;
    }
    SDL_Surface *PngLibrary::load_png(SDL_RWops *rwops) const{
        if(not is_png(rwops)){
            return nullptr;
        }
        //Reset to begin
        SDL_RWseek(rwops,0,RW_SEEK_SET);


        png_structp ptr = nullptr;
        png_infop info = nullptr;
        SDL_Surface *surf = nullptr;
        ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            nullptr,
            nullptr,
            nullptr
        );
        info = png_create_info_struct(ptr);

        png_set_read_fn(ptr,rwops,read_callback);

        png_uint_32 w,h;
        int depth;
        int color_type;

        PNG_TRY(ptr){
            //Skip the byte
            png_set_sig_bytes(ptr,8);
            png_read_info(ptr,info);
            //png_get_IHDR();
            png_get_IHDR(ptr,info,&w,&h,&depth,&color_type,nullptr,nullptr,nullptr);
        }
        PNG_CATCH{
            goto err;
        }
        surf = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            w,
            h,
            depth,
            0,
            0,
            0,
            0
        );
        if(surf == nullptr){
            goto err;
        }
        png_destroy_read_struct(&ptr,&info,nullptr);
        return surf;
        err:
            png_destroy_read_struct(&ptr,&info,nullptr);
            return nullptr;
    }
}
namespace Btk{
    void RegisterPNG(){
        ImageAdapter adapter;
        adapter.name = "png";
        adapter.fn_is = [](SDL_RWops *r){
            pnglib.load();
            return pnglib->is_png(r);
        };
        adapter.fn_load = [](SDL_RWops *r){
            pnglib.load();
            return pnglib->load_png(r);
        };
        RegisterImageAdapter(adapter);
    }
}