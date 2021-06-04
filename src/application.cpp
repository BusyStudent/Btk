#include "./build.hpp"

#include <SDL2/SDL_clipboard.h>

#include <Btk/impl/application.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/application.hpp>
#include <Btk/Btk.hpp>

namespace Btk{
    Application::Application(){
        Init();
        app = new ApplicationImpl;    
    }
    Application::~Application(){
        HasSlots::cleanup();
        delete app;
    }
    Signal<void()> &Application::signal_quit(){
        return Instance().signal_quit;
    }
    int Application::run(){
        
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
