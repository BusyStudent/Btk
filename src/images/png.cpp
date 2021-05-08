#include "../build.hpp"

#include <Btk/impl/loadso.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/rwops.hpp>
#include <csetjmp>
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

namespace Btk{
    struct PngLibrary{
        BTK_PNG_LIBRARY(PNG_LIBNAME);

        BTK_PNG_FUNCTION(png_create_info_struct);
        BTK_PNG_FUNCTION(png_create_read_struct);
        BTK_PNG_FUNCTION(png_destroy_info_struct);
        BTK_PNG_FUNCTION(png_destroy_read_struct);

        BTK_PNG_FUNCTION(png_set_longjmp_fn);
        BTK_PNG_FUNCTION(png_set_longjmp_fn);
        BTK_PNG_FUNCTION(png_set_sig_bytes);
        BTK_PNG_FUNCTION(png_error);
    };
}


#define PNG_TRY(CTXT) if(std::setjmp(png_jmpbuf(CTXT)) == 0)
#define PNG_CATCH else

namespace Btk{
    static void read_callback(png_structp png,png_bytep buf,size_t size){
        void *rw = png_get_io_ptr(png);
        size_t n = SDL_RWread(static_cast<SDL_RWops*>(rw),buf,1,size);
        //failure
        if(n == 0){
            png_error(png,SDL_GetError());
        }
    }
    static bool is_png(SDL_RWops *rwops){
        constexpr int PNG_SIG_BYTE = 8;
        char buf[PNG_SIG_BYTE];

        SDL_RWseek(rwops,0,RW_SEEK_SET);

        if(SDL_RWread(rwops,buf,1,PNG_SIG_BYTE) != PNG_SIG_BYTE){
            //FAILED to read
            return false;
        }
        return png_sig_cmp(buf,0,PNG_SIG_BYTE)== 0;
    }
    static SDL_Surface *read_png(SDL_RWops *rwops){
        if(not is_png(rwops)){
            return nullptr;
        }
        //Reset to begin
        SDL_RWseek(rwops,0,RW_SEEK_SET);


        png_structp ptr;
        ptr = png_create_read_struct(
            PNG_LIBPNG_VER_STRING,
            nullptr,
            nullptr,
            nullptr
        );
        png_infop info = png_create_info_struct(ptr);

        png_set_read_fn(ptr,rwops,read_callback);

        int w,h;

        PNG_TRY(ptr){
            //Skip the byte
            png_set_sig_bytes(ptr,8);
            png_read_info(ptr,info);
            //png_get_IHDR();
        }
        PNG_CATCH{
            png_destroy_read_struct(&ptr,&info,nullptr);
            return nullptr;
        }

        png_destroy_read_struct(&ptr,&info,nullptr);
    }
}