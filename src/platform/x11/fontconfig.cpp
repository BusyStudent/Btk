#include <SDL2/SDL_ttf.h>
#include <fontconfig/fontconfig.h>

#include <Btk/platform/x11.hpp>
#include <Btk/exception.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    //X11 Font System Impl
namespace X11{
    static bool was_init = false;
    void InitFont(){
        if(not was_init){
            if(FcInit() == FcFalse){
                //Init fail
                throwRuntimeError("FcInit() failed");
            }
            was_init = true;
        }
    };
    void QuitFont(){
        if(was_init){
            FcFini();
            was_init = false;
        }
    };
};
};