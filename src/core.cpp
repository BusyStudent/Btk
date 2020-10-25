#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_filesystem.h>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <unordered_map>

#include <Btk/impl/window.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/defs.hpp>
#include <Btk/Btk.hpp>
namespace{
    void defer_call_cb(SDL_Event &ev,void *){
        //defer call
        typedef void (*defer_fn_t)(void*);
        defer_fn_t fn = reinterpret_cast<defer_fn_t>(ev.user.data1);
        #ifndef NDEBUG
            SDL_Log("[System::EventDispather]call %p data = %p",fn,ev.user.data2);
        #endif
        fn(ev.user.data2);
    };
};
namespace Btk{
    System* System::instance = nullptr;
    bool    System::is_running = false;
    void Init(){
        //Init Btk
        System::Init();
    }
    int Main(){
        static std::thread::id thid = std::this_thread::get_id();
       
        if(thid != std::this_thread::get_id() or System::is_running == true){
            //double call or call from another thread
            return -1;
        }
        Btk_defer{
            //Quit main
            System::is_running = false;
        };
        //resume from error
        resume:System::is_running = true;
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
            throw;
        }
        catch(...){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException]Unknown");
            if(System::instance->handle_exception != nullptr){
                if(System::instance->handle_exception(nullptr)){
                    goto resume;
                }
            }
            throw;
        }
        return 0;
    }
    System::System(){
        defer_call_ev_id = SDL_RegisterEvents(1);
        if(defer_call_ev_id == (Uint32)-1){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Could not regitser event");
        }
        else{
            //regitser handler
            regiser_eventcb(defer_call_ev_id,defer_call_cb,nullptr);
        }
    }
    System::~System(){
        //run all exit handlers
        for(auto &handler:atexit_handlers){
            handler();
        }
    }
    //Init or Quit
    int  System::Init(){
        if(instance == nullptr){
            if(SDL_Init(SDL_INIT_VIDEO) == -1){
                //Failed to init
                return -1;
            }
            #ifndef NDEBUG
            //show detail version
            SDL_version ver;
            const SDL_version *iver = IMG_Linked_Version();
            SDL_GetVersion(&ver);
            SDL_Log("[System::Core]Init SDL2 Platfrom %s",SDL_GetPlatform());
            SDL_Log("[System::Core]SDL2 version: %d.%d.%d",ver.major,ver.major,ver.patch);
            SDL_Log("[System::Core]SDL2 image version: %d.%d.%d",iver->major,iver->major,iver->patch);
            #endif
            //Create instance
            instance = new System();
            //regitser atexit callback
            std::atexit(System::Quit);
        }
        return 1;
    }
    //Global Cleanup
    void System::Quit(){
        if(instance->is_running){
            abort();
        }
        //delete instance to cleanup windows
        delete instance;
        instance = nullptr;
        //Quit SDL
        IMG_Quit();
        //TTF_Quit();
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
                        if(win.second->on_close()){
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
                case SDL_MOUSEMOTION:{
                    on_mousemotion(event);
                    break;
                }
                default:{
                    //get function from event callbacks map
                    auto iter = evcbs_map.find(event.type);
                    if(iter != evcbs_map.end()){
                        iter->second(event);
                    }
                    else{
                        SDL_LogWarn(
                            SDL_LOG_CATEGORY_APPLICATION,
                            "[System::EventDispather]unknown event id %d timestamp %d",
                            event.type,
                            SDL_GetTicks()
                        );
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
                if(win->on_close()){
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
            case SDL_WINDOWEVENT_RESIZED:{
                //window resize
                win->on_resize(event.window.data1,event.window.data2);
                break;
            }
        }
    }
    void System::on_mousemotion(SDL_Event &){
        //....
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
        win->on_dropfile(event.drop.file);
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
            SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM,"[System::WindowsManager]cannot get window %d",winid);
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
    //register a exit handler
    void System::atexit(void (*fn)(void *),void *data){
        atexit_handlers.push_back({
            fn,
            data
        });
    }
    //std c handlers
    void System::atexit(void (*fn)()){
        System::atexit([](void *fn){
            (reinterpret_cast<void(*)()>(fn))();
        },reinterpret_cast<void*>(fn));
    }
    //event callbacks cb
    void System::regiser_eventcb(Uint32 evid,EventHandler::FnPtr ptr,void *data){
        evcbs_map[evid] = {
            ptr,
            data
        };
    }
    WindowImpl *System::get_window(Uint32 winid){
        auto iter = wins_map.find(winid);
        if(iter == wins_map.end()){
            //Warn
            //maybe the window is closed
            SDL_LogWarn(SDL_LOG_CATEGORY_SYSTEM,"[System::WindowsManager]cannot get window %d",winid);
            return nullptr;
        }
        else{
            return iter->second;
        }
    }
    void Exit(int code){
        //FIXME possible memory leak on here
        int *value = new int(code);
        System::instance->defer_call([](void *ptr){
            int v = *static_cast<int*>(ptr);
            delete ptr;
            throw v;
        },value);
    }
    ExceptionHandler SetExceptionHandler(ExceptionHandler handler){
        ExceptionHandler current = System::instance->handle_exception;
        System::instance->handle_exception = handler;
        return current;
    }
};