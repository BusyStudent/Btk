#include <SDL2/SDL_ttf.h>

#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/platform.hpp>
#include <Btk/Btk.hpp>
#ifdef _WIN32
    #include <Btk/platform/win32.hpp>
#else
    #include <Btk/platform/x11.hpp>
#endif
#include <cstdlib>
#include <mutex>
namespace Btk{
namespace Platform{
    void InitFont(){
        static std::once_flag flag;
        //First init SDL_ttf
        if(TTF_Init() == -1){
            //Error
            throwSDLError();
        }
        #ifdef _WIN32
        Win32::InitFont();
        #else
        X11::InitFont();
        #endif
        std::call_once(flag,[](){
            //Init System for Btk::AtExit
            System::Init();
            AtExit(QuitFont);
        });
    };
    void QuitFont(){
        TTF_Quit();
        #ifdef _WIN32
        Win32::QuitFont();
        #else
        X11::QuitFont();
        #endif
    };
};
};