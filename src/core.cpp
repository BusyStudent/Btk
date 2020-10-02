#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <unordered_map>

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/defs.hpp>
#include <Btk/Btk.hpp>
#define Btk_defer ScopeGuard __GUARD__ = [&]()
namespace{
    //ScopeGuard
    template<class T>
    struct ScopeGuard{
        inline ScopeGuard(T &&f):fn(f){}
        inline ~ScopeGuard(){
            fn();
        }
        T &fn;
    };
};
namespace Btk{
    System* System::instance = nullptr;
    void Init(){
        //Init Btk
        static std::once_flag flag;
        std::call_once(flag,[](){
            System::instance = new System();
            std::atexit([](){
                delete System::instance;
            });
        });
    }
    int Main(){
        static std::thread::id thid = std::this_thread::get_id();
        static bool is_running = false;
        if(thid != std::this_thread::get_id() or is_running == true){
            //double call or call from another thread
            return -1;
        }
        //resume from error
        resume:is_running = true;
        try{
            
            System::instance->run();
        }
        catch(int i){
            //Exit
            return i;
        }
        catch(std::exception &exp){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException] %s",exp.what());
            //call handler
            if(System::instance->handle_exception != nullptr){
                if(System::instance->handle_exception(&exp)){
                    goto resume;
                }
            }
            std::abort();
        }
        catch(...){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException]Unknown");
            if(System::instance->handle_exception != nullptr){
                if(System::instance->handle_exception(nullptr)){
                    goto resume;
                }
            }
            std::abort();
        }
        return 0;
    }
    System::System(){
        SDL_Init(SDL_INIT_VIDEO);
        defer_call_ev_id = SDL_RegisterEvents(1);
        if(defer_call_ev_id == (Uint32)-1){
            SDL_LogError(SDL_LOG_CATEGORY_ERROR,"Could not regitser event");
        }
        #ifndef NDEBUG
        SDL_Log("[System::Core]Init SDL2 Platfrom %s",SDL_GetPlatform());
        #endif
    }
    System::~System(){
        IMG_Quit();
        SDL_Quit();
        
    }
    //EventLoop
    void System::run(){
        SDL_Event event;
        while(SDL_WaitEvent(&event)){
            switch(event.type){
                case SDL_QUIT:{
                    #ifndef NDEBUG
                    SDL_Log("[System::EventDispather] Got SDL_QUIT");
                    #endif
                    std::lock_guard<std::recursive_mutex> locker(map_mtx);
                    for(auto &win:wins_map){
                        if(win.second->close()){
                            //Close Window
                            
                            unregister_window(win.second);
                        }
                    }
                    if(wins_map.empty()){
                        return;
                    }
                    break;
                }
                case SDL_WINDOWEVENT:{
                    on_windowev(event);
                    break;
                }
                case SDL_DROPFILE:{
                    #ifndef NDEBUG
                    SDL_Log("[System::EventDispather] Got SDL_DROPFILE");
                    #endif
                    on_dropev(event);
                    break;
                }
                default:{
                    if(event.type == defer_call_ev_id){
                        //defer call
                        typedef void(*defer_fn_t)(void*);
                        defer_fn_t fn = reinterpret_cast<defer_fn_t>(event.user.data1);
                        #ifndef NDEBUG
                        SDL_Log("[System::EventDispather]call %p data = %p",fn,event.user.data2);
                        #endif
                        fn(event.user.data2);
                    }
                }
            }
        }
    }
    //WindowEvent
    void System::on_windowev(SDL_Event &event){
        WindowImpl *win;
        std::lock_guard<std::recursive_mutex> locker(map_mtx);
        win = get_window(event.window.windowID);
        if(win == nullptr){
            return;
        }


        switch(event.window.event){
            case SDL_WINDOWEVENT_EXPOSED:{
                //redraw window
                win->draw();
                break;
            }
            case SDL_WINDOWEVENT_CLOSE:{
                //Close window;
                if(win->close()){
                    //success to close
                    //Delete Wnidow;
                    unregister_window(win);
                    if(wins_map.empty()){
                        //No window exist
                        Btk::Exit(0);
                    }
                }
                break;
            }
        }
    }
    //DropFile
    void System::on_dropev(SDL_Event &event){
        Btk_defer{
            SDL_free(event.drop.file);
        };
        WindowImpl *win;
        std::lock_guard<std::recursive_mutex> locker(map_mtx);
        win = get_window(event.drop.windowID);
        if(win == nullptr){
            return;
        }
        win->dropfile(event.drop.file);
    }
    void System::register_window(WindowImpl *impl){
        if(impl == nullptr){
            return;
        }
        std::lock_guard<std::recursive_mutex> locker(map_mtx);
        Uint32 winid = SDL_GetWindowID(impl->win);
        //GetWindowId
        wins_map[winid] = impl;
        //add refcount
        impl->ref();
    }
    void System::unregister_window(WindowImpl *impl){
        if(impl == nullptr){
            return;
        }
        std::lock_guard<std::recursive_mutex> locker(map_mtx);
        Uint32 winid = SDL_GetWindowID(impl->win);
        
        auto iter = wins_map.find(winid);
        if(iter == wins_map.end()){
            //Handle err
        }
        else{
            //erase it
            iter->second->unref();
            iter->second = nullptr;
            wins_map.erase(iter);
        }
    }
    //Push a defercall event
    void System::defer_call(void(* fn)(void*),void* data){
        SDL_Event event;
        SDL_zero(event);
        event.type = defer_call_ev_id;
        event.user.data1 = reinterpret_cast<void*>(fn);
        event.user.data2 = data;
        SDL_PushEvent(&event);
    }
    WindowImpl *System::get_window(Uint32 winid){
        auto iter = wins_map.find(winid);
        if(iter == wins_map.end()){
            //Error
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System]:cannot get window %d",winid);
            return nullptr;
        }
        else{
            return iter->second;
        }
    }
    void Exit(int code){
        System::instance->defer_call([](void*){
            throw int(1);
        });
    }
    ExceptionHandler SetExceptionHandler(ExceptionHandler handler){
        ExceptionHandler current = System::instance->handle_exception;
        System::instance->handle_exception = handler;
        return current;
    }
};