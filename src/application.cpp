#include "./build.hpp"

#include <SDL2/SDL_clipboard.h>

#include <Btk/detail/application.hpp>
#include <Btk/detail/core.hpp>
#include <Btk/application.hpp>
#include <Btk/Btk.hpp>

namespace Btk{
    Application::Application(){
        Init();
    }
    Application::~Application(){
        HasSlots::cleanup();
        Quit();
    }
    Signal<void()> &Application::signal_quit(){
        return GetSystem()->signal_quit;
    }
    int Application::run(){
        return Btk::run();
    }
    bool Application::mainloop(){
        return run() == 0;
    }
    void Application::set_clipboard(const u8string &s){
        if(SDL_SetClipboardText(s.c_str()) != 0){
            throwSDLError();
        }
    }
    
}
