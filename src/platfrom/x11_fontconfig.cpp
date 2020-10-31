#include <SDL2/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#include <Btk/impl/core.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    //X11 Font System Impl
    static bool font_inited = false;
    void QuitFontSystem(){
        FcFini();
        TTF_Quit();
    }
    int  InitFontSystem(){
        //try init TTF
        if(font_inited){
            return 0;
        }
        //Init Sytem
        System::Init();
        if(TTF_Init() == -1 or FcInit() == FcFalse){
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"[System::TTF]Init failed");
            return -1;
        }
        else{
            AtExit(QuitFontSystem);
        }
        return 0;
    }
};