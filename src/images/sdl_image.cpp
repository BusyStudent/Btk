#include "../build.hpp"

#include <Btk/detail/core.hpp>
#include <Btk/Btk.hpp>
#include "adapter.hpp"
#include <SDL2/SDL_image.h>
#define BTK_SDLIMG_WRAPPER(NAME) { \
    Btk::ImageAdapter adapter;\
    adapter.name = #NAME;\
    adapter.fn_save = nullptr;\
    adapter.fn_load = IMG_Load##NAME##_RW;\
    adapter.fn_is = [](SDL_RWops *rwops) -> bool{\
        return IMG_is##NAME(rwops);\
    };\
    adapter.vendor = "sdlimage";\
    Btk::RegisterImageAdapter(adapter);\
}
namespace Btk{
    //Register function in SDL_image
    void RegisterSDLImage(){
        BTK_SDLIMG_WRAPPER(ICO);
        BTK_SDLIMG_WRAPPER(CUR);
        BTK_SDLIMG_WRAPPER(JPG);
        BTK_SDLIMG_WRAPPER(BMP);
        BTK_SDLIMG_WRAPPER(GIF);
        BTK_SDLIMG_WRAPPER(LBM);
        BTK_SDLIMG_WRAPPER(PNG);
        BTK_SDLIMG_WRAPPER(PCX);
        BTK_SDLIMG_WRAPPER(PNM);
        BTK_SDLIMG_WRAPPER(SVG);
        BTK_SDLIMG_WRAPPER(TIF);
        BTK_SDLIMG_WRAPPER(XCF);
        BTK_SDLIMG_WRAPPER(XPM);
        BTK_SDLIMG_WRAPPER(WEBP);
        BTK_SDLIMG_WRAPPER(XV);
        AtExit(IMG_Quit);
        
        #ifndef NDEBUG
        const SDL_version *iver = IMG_Linked_Version();
        SDL_Log("[System::IMG]SDL2 image version: %d.%d.%d",iver->major,iver->major,iver->patch);
        #endif
    }
}
