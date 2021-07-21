#include <SDL2/SDL.h>
#include <SDL2/SDL_loadso.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_filesystem.h>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <mutex>
#include <unordered_map>

#include "build.hpp"

#include <Btk/platform/platform.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/impl/thread.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/module.hpp>
#include <Btk/async.hpp>
#include <Btk/gl/gl.hpp>
#include <Btk/mixer.hpp>
#include <Btk/event.hpp>
#include <Btk/defs.hpp>
#include <Btk/Btk.hpp>


namespace{
    void defer_call_cb(const SDL_Event &ev,void *){
        //defer call
        typedef void (*defer_fn_t)(void*);
        defer_fn_t fn = reinterpret_cast<defer_fn_t>(ev.user.data1);
        #ifndef NDEBUG
            SDL_Log("[System::EventDispather]call %p data = %p",fn,ev.user.data2);
        #endif
        fn(ev.user.data2);
    }
    /**
     * @brief A wrapper to call function pointer void(*)()
     * 
     * @param fn function pointer
     */
    void callback_wrapper(void *fn){
        auto f = reinterpret_cast<void(*)()>(fn);
        if(f != nullptr){
            f();
        }
    }
}
namespace Btk{
    //< The thread id of thread which called Btk::run
    static std::thread::id main_thrd;
    System* System::instance = nullptr;
    bool    System::is_running = false;
    void Init(){
        //Init Btk
        System::Init();
    }
    int  run(){
        if(main_thrd == std::thread::id()){
            main_thrd = std::this_thread::get_id();
        }
       
        if(main_thrd != std::this_thread::get_id() or System::is_running == true){
            //double call or call from another thread
            return -1;
        }
        //resume from error
        resume:System::is_running = true;
        int retvalue = EXIT_SUCCESS;
        try{
            System::instance->run();
        }
        catch(int i){
            //Exit
            retvalue = i;
        }
        catch(std::exception &exp){
            
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException] %s",exp.what());
            if(Instance().try_handle_exception(&exp)){
                goto resume;
            }
            Instance().is_running = false;
            throw;
        }
        catch(...){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException]Unknown");
            if(Instance().try_handle_exception(nullptr)){
                goto resume;
            }
            Instance().is_running = false;
            throw;
        }
        Instance().is_running = false;
        return retvalue;
    }
    System::System(){
        AsyncInit();
        defer_call_ev_id = SDL_RegisterEvents(2);
        if(defer_call_ev_id == (Uint32)-1){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Could not regitser event");
        }
        else{
            //regitser handler
            regiser_eventcb(defer_call_ev_id,defer_call_cb,nullptr);
        }
        //Builtin BMP Adapter
        ImageAdapter adapter;
        adapter.name = "bmp";
        adapter.fn_load = [](SDL_RWops *rwops){
            return SDL_LoadBMP_RW(rwops,SDL_FALSE);
        };
        adapter.fn_save = [](SDL_RWops *rwops,SDL_Surface *s,int) -> bool{
            return SDL_SaveBMP_RW(s,rwops,SDL_FALSE);
        };
        adapter.fn_is = [](SDL_RWops *rwops) -> bool{
            //Check magic is bm
            char magic[2] = {0};
            //Save cur
            auto cur = SDL_RWtell(rwops);
            //Read magic
            SDL_RWread(rwops,magic,sizeof(magic),1);
            //Reset to the position
            SDL_RWseek(rwops,cur,RW_SEEK_SET);
            //Check magic
            return magic[0] == 'B' and magic[1] == 'M';
        };
        image_adapters.emplace_back(adapter);

        //First stop text input
        SDL_StopTextInput();
    }
    System::~System(){
        AsyncQuit();
        //run all exit handlers
        for(auto &handler:atexit_handlers){
            handler();
        }
        //unload modules
        for(auto &mod:modules_list){
            mod.unload();
        }
    }
    //Init or Quit
    int  System::Init(){
        static std::once_flag flag;
        if(instance == nullptr){
            if(SDL_Init(SDL_INIT_VIDEO) == -1){
                //Failed to init
                return -1;
            }
            // if(TTF_Init() == -1){
            //     //Failed to init font engine
            //     return -1;
            // }
            SDL_EnableScreenSaver();
            #ifndef NDEBUG
            //show detail version
            SDL_version ver;
            SDL_GetVersion(&ver);
            SDL_Log("[System::Core]Init SDL2 Platfrom %s",SDL_GetPlatform());
            SDL_Log("[System::Core]SDL2 version: %d.%d.%d",ver.major,ver.major,ver.patch);
            #endif
            //Create instance
            instance = new System();
            //regitser atexit callback
            std::call_once(flag,std::atexit,System::Quit);
            //Init platform
            Platform::Init();
            GL::Init();
            //Image Adapter
            InitImageAdapter();
        }
        return 1;
    }
    //Global Cleanup
    void System::Quit(){
        BTK_ASSERT(System::instance->is_running == false);
        //delete instance to cleanup windows
        delete instance;
        instance = nullptr;
        //Cleanup platform
        Platform::Quit();
        //Quit SDL
        // TTF_Quit();
        SDL_Quit();
    }
    //EventLoop
    void System::run(){
        SDL_Event event;
        while(SDL_WaitEvent(&event)){
            switch(event.type){
                case SDL_QUIT:{
                    on_quit();
                    break;
                }
                case SDL_CLIPBOARDUPDATE:{
                    BTK_LOGINFO("[System::Core]Emitting SignalClipboardUpdate");
                    signal_clipboard_update();
                    break;
                }
                case SDL_WINDOWEVENT:{
                    on_windowev(event);
                    break;
                }
                case SDL_DROPCOMPLETE:
                case SDL_DROPBEGIN:
                case SDL_DROPTEXT:
                case SDL_DROPFILE:{
                    on_dropev(event);
                    break;
                }
                case SDL_MOUSEMOTION:{
                    on_mousemotion(event);
                    break;
                }
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEBUTTONDOWN:{
                    on_mousebutton(event);
                    break;
                }
                case SDL_KEYUP:
                case SDL_KEYDOWN:{
                    on_keyboardev(event);
                    break;
                }
                case SDL_TEXTINPUT:{
                    on_textinput(event);
                    break;
                }
                case SDL_MOUSEWHEEL:{
                    on_mousewheel(event);
                    break;
                }
                case SDL_SYSWMEVENT:{
                    Platform::HandleSysMsg(*(event.syswm.msg));
                    break;
                }
                case SDL_AUDIODEVICEADDED:{
                    BTK_LOGINFO("[System::Audio] AudioDeviceAdded");
                    signal_audio_device_added();
                    break;
                }
                case SDL_AUDIODEVICEREMOVED:{
                    BTK_LOGINFO("[System::Audio] AudioDeviceRemoved");
                    signal_audio_device_removed();
                    break;
                }
                case SDL_KEYMAPCHANGED:{
                    signal_keymap_changed();
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
        BTK_LOGWARN(SDL_GetError());

    }
    //WindowEvent
    inline void System::on_windowev(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.window.windowID);
        if(win == nullptr){
            return;
        }
        win->handle_windowev(event);
    }
    //MouseMotion
    inline void System::on_mousemotion(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.motion.windowID);
        if(win == nullptr){
            return;
        }
        auto motion = TranslateEvent(event.motion);
        win->handle_motion(motion);
    }
    //MouseButton
    inline void System::on_mousebutton(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.button.windowID);
        if(win == nullptr){
            return;
        }
        auto click = TranslateEvent(event.button);
        win->handle_mouse(click);
    }
    //MouseWheel
    inline void System::on_mousewheel(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.button.windowID);
        if(win == nullptr){
            return;
        }
        auto wheel = TranslateEvent(event.wheel);
        if(win->handle_wheel(wheel)){
            if(not wheel.is_accepted()){
                win->sig_event(wheel);
            }
        }
    }
    //DropFile
    inline void System::on_dropev(const SDL_Event &event){
        WindowImpl *win;
        //auto free it
        SDLScopePtr ptr(event.drop.file);
        win = get_window_s(event.drop.windowID);
        if(win == nullptr){
            return;
        }
        //Translate event and send
        auto drop = TranslateEvent(event.drop);
        win->handle_drop(drop);
    }
    //KeyBoardEvent
    inline void System::on_keyboardev(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.key.windowID);
        if(win == nullptr){
            return;
        }
        auto kevent = TranslateEvent(event.key);
        if(not win->handle_keyboard(kevent)){
            //No one process it
            if(not kevent.is_accepted()){
                win->sig_event(kevent);
            }
        }
    }
    inline void System::on_textinput(const SDL_Event &event){
        //Get text input
        TextInputEvent ev;
        ev.text = event.text.text;
        WindowImpl *win = get_window_s(event.text.windowID);
        if(win == nullptr){
            return;
        }
        win->handle_textinput(ev);
    }
    inline void System::on_quit(){
        if(signal_quit.empty()){
            //Deafult close all the window and quit
            BTK_LOGINFO("[System::Core]Try to quit");
            std::lock_guard<std::recursive_mutex> locker(map_mtx);
            for(auto i = wins_map.begin(); i != wins_map.end();){
                //Try to close window
                if(i->second->on_close()){
                    //Succeed
                    BTK_LOGINFO("[System::Core]Succeed to close %p",i->second);
                    signal_window_closed(i->second);
                    delete i->second;
                    i = wins_map.erase(i);
                }
                else{
                    ++i;
                }
            }
            if(wins_map.empty()){
                Exit();
            }
        }
        else{
            BTK_LOGINFO("[System::Core]Emitting SignalQuit");
            signal_quit();
        }
    }
    void System::register_window(WindowImpl *impl){
        if(impl == nullptr){
            return;
        }
        std::lock_guard<std::recursive_mutex> locker(map_mtx);
        Uint32 winid = SDL_GetWindowID(impl->win);
        //GetWindowId
        wins_map.insert(std::make_pair(winid,impl));
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
            delete iter->second;
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
        event.user.timestamp = SDL_GetTicks();
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
        System::atexit(callback_wrapper,reinterpret_cast<void*>(fn));
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
    WindowImpl *System::get_window_s(Uint32 winid){
        std::lock_guard locker(map_mtx);
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
    bool System::try_handle_exception(std::exception *exp){
        if(handle_exception == nullptr){
            return false;
        }
        return handle_exception(exp);
    }
    void System::close_window(WindowImpl *win){
        if(win == nullptr){
            return;
        }
        if(win->on_close()){
            signal_window_closed(win);
            unregister_window(win);
        }
        if(wins_map.empty()){
            Exit(EXIT_SUCCESS);
        }
    }
    WindowImpl *System::create_window(SDL_Window *win){
        BTK_ASSERT(win != nullptr);
        auto p = new WindowImpl(win);
        register_window(p);
        SDL_SetWindowData(win,"btk_imp",p);
        signal_window_created(p);
        return p;
    }
    RendererDevice *System::create_device(SDL_Window *w){
        Device *dev;
        for(auto fn:devices_list){
            dev = fn(w);
            if(dev != nullptr){
                return dev;
            }
        }
        return nullptr;
    }
}
namespace Btk{
    static inline void exit_impl_cb(void *args){
        int v = *reinterpret_cast<int*>(&args);
        throw v;
    }
    void Exit(int code){
        if constexpr(sizeof(void *) < sizeof(int)){
            //FIXME possible memory leak on here
            DeferCall([code](){
                throw code;
            });
        }
        else{
            void *args;
            *reinterpret_cast<int*>(&args) = code;
            DeferCall(exit_impl_cb,args);
        }
    }
    ExceptionHandler SetExceptionHandler(ExceptionHandler handler){
        ExceptionHandler current = System::instance->handle_exception;
        System::instance->handle_exception = handler;
        return current;
    }
    void AtExit(void(* fn)(void*),void *data){
        BTK_ASSERT(System::instance != nullptr);

        System::instance->atexit(fn,data);
    }
    void AtExit(void(* fn)()){
        System::instance->atexit(fn);
    }
    void DeferCall(void(*fn)(void*),void *userdata){
        BTK_ASSERT(System::instance != nullptr);
        
        System::instance->defer_call(
            fn,userdata
        );
    }
    void DeferCall(void(*fn)()){
        System::instance->defer_call(
            callback_wrapper,reinterpret_cast<void*>(fn)
        );
    }
    bool IsMainThread(){
        return main_thrd == std::this_thread::get_id();
    }
    bool CouldBlock(){
        return not Instance().is_running or not IsMainThread();
    }
    void RegisterImageAdapter(const ImageAdapter & a){
        Instance().image_adapters.push_front(a);
        BTK_LOGINFO("[System::Core]Register ImageAdapter %s",a.name);
    }
    void RegisterDevice(System::CreateDeviceFn fn){
        if(fn == nullptr){
            return;
        }
        Instance().devices_list.emplace_front(fn);
    }
    SDL_Surface *LoadImage(SDL_RWops *rwops,u8string_view type){
        BTK_ASSERT(rwops != nullptr);
        SDL_Surface *ret;
        if(type.empty()){
            //Didnot provide the type
            for(auto &adapter:Instance().image_adapters){
                if(adapter.fn_is == nullptr){
                    //It cannot check the type
                    //Try load it directly
                    ret = adapter.load(rwops);
                }
                //Is the type
                else if(adapter.is(rwops)){
                    ret = adapter.load(rwops);
                }
                else{
                    continue;
                }
                if(ret == nullptr){
                    throwRuntimeError(SDL_GetError());
                }
                else{
                    return ret;
                }
            }
            throwRuntimeError("Unsupport format");
        }
        SDL_SetError("Unsupport format");
        for(auto &adapter:Instance().image_adapters){
            if(SDL_strncasecmp(adapter.name,type.data(),type.size()) == 0){
                ret = adapter.load(rwops);
                if(ret == nullptr){
                    continue;
                }
                else{
                    return ret;
                }
            }
        }
        throwRuntimeError(SDL_GetError());
    }
};
namespace Btk{
    void Module::unload(){
        if(quit != nullptr){
            quit(*this);
        }
        SDL_UnloadObject(handle);
    }
    void LoadModule(u8string_view module_name){
        void *handle = SDL_LoadObject(module_name.data());
        if(handle == nullptr){
            throwSDLError();
        }
        auto init_fn = reinterpret_cast<Module::InitFn>(
            SDL_LoadFunction(handle,"BtkModule_Init")
        );
        if(init_fn == nullptr){
            auto err = SDL_GetError();
            SDL_UnloadObject(handle);
            throwRuntimeError(err);
        }
        auto quit_fn = reinterpret_cast<Module::QuitFn>(
            SDL_LoadFunction(handle,"BtkModule_Quit")
        );
        Module mod;
        mod.handle = handle;
        mod.init = init_fn;
        mod.quit = quit_fn;

        try{
            init_fn(mod);
        }
        catch(...){
            SDL_UnloadObject(handle);
            throw;
        }

        Instance().modules_list.push_back(mod);
    }
    bool HasModule(u8string_view name){
        for(auto &mod:Instance().modules_list){
            if(mod.name == name){
                return true;
            }
        }
        return false;
    }
}
namespace Btk{
    static thread_local u8string u8buf; 
    static thread_local u16string u16buf;

    u8string&  InternalU8Buffer(){
        return u8buf;
    }
    u16string& InternalU16Buffer(){
        return u16buf;
    }
}