#include "../build.hpp"
#include <Btk/detail/core.hpp>
#include "adapter.hpp"

#ifndef BTK_HAS_SDLIMG
    #include "xpm.cpp"
#endif


namespace Btk{
    void InitImageAdapter(){
        RegisterSDLImage();
        RegisterSTBII();
        RegisterWEBP();
        RegisterWIC();
        RegisterPNG();
        RegisterGIF();

        #ifndef BTK_HAS_SDLIMG
        ImageAdapter adapter;
        adapter.name = "xpm";
        adapter.vendor = "bultin";
        adapter.fn_is = BultinIsXPM;
        adapter.fn_load = load_xpm_from_string;
        adapter.fn_save = save_xpm_from_buf;
        RegisterImageAdapter(adapter);
        #endif
    }
}