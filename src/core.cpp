#include <SDL2/SDL.h>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <unordered_map>

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>

namespace Btk{
    System* System::current = nullptr;
    void Init(){
        //Init Btk
        static std::once_flag flag;
        std::call_once(flag,[](){
            System::current = new System();
            std::atexit([](){
                delete System::current;
            });
        });
    }
    int Main(){
        static std::thread::id thid = std::this_thread::get_id();
        if(thid != std::this_thread::get_id()){
            //double call
            return -1;
        }
        else{
            System::current->run();
            return 0;
        }
    }
    System::System(){
        SDL_Init(SDL_INIT_VIDEO);
    }
    System::~System(){
        SDL_Quit();
    }
    void System::run(){
        SDL_Event event;
        while(SDL_WaitEvent(&event)){
            if(event.type == SDL_QUIT){
                return;
            }
        }
    }
};