#include "../build.hpp"
#include <Btk/detail/core.hpp>
#include "adapter.hpp"

#ifndef BTK_HAS_SDLIMG
    #include "xpm.cpp"
#endif

#define ADAPTER_BEGIN {
#define ADAPTER_END }

namespace Btk{
    void InitImageAdapter(){
        RegisterSDLImage();
        RegisterSTBII();
        RegisterWEBP();
        RegisterWIC();
        RegisterPNG();
        RegisterGIF();

        #ifndef BTK_HAS_SDLIMG
        ADAPTER_BEGIN
        ImageAdapter adapter;
        adapter.name = "xpm";
        adapter.vendor = "bultin";
        adapter.fn_is = BultinIsXPM;
        adapter.fn_load = load_xpm_from_string;
        adapter.fn_save = save_xpm_from_buf;
        RegisterImageAdapter(adapter);
        ADAPTER_END
        #endif

        ADAPTER_BEGIN
        //Builtin BMP Adapter
        ImageAdapter adapter;
        adapter.name = "bmp";
        adapter.vendor = "bultin";
        adapter.fn_load = [](SDL_RWops *rwops){
            return SDL_LoadBMP_RW(rwops,SDL_FALSE);
        };
        adapter.fn_save = [](SDL_RWops *rwops,SDL_Surface *s,int) -> bool{
            return SDL_SaveBMP_RW(s,rwops,SDL_FALSE) == 0;
        };
        adapter.fn_is = BultinIsBMP;
        RegisterImageAdapter(adapter);
        ADAPTER_END
    }
}