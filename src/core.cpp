#include <SDL2/SDL.h>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <mutex>

#include "build.hpp"

#include <Btk/platform/platform.hpp>
#include <Btk/detail/window.hpp>
#include <Btk/detail/thread.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/detail/core.hpp>
#include <Btk/graphics/opengl.hpp>
#include <Btk/exception.hpp>
#include <Btk/module.hpp>
#include <Btk/async.hpp>
#include <Btk/mixer.hpp>
#include <Btk/event.hpp>
#include <Btk/defs.hpp>
#include <Btk/Btk.hpp>

#ifdef BTK_HAVE_SOFTWARE_DEVICE
    //Enable Software Device
    #include <Btk/graphics/software.hpp>
#endif



namespace{
    void defer_call_cb(const SDL_Event &ev,void *){
        //defer call
        typedef void (*defer_fn_t)(void*);
        defer_fn_t fn = reinterpret_cast<defer_fn_t>(ev.user.data1);
        
        BTK_LOGINFO("[System::EventDispather]call %p data = %p",fn,ev.user.data2);
        
        fn(ev.user.data2);
    }
    void redraw_win_cb(const SDL_Event &ev,void *){
        auto w = Btk::GetSystem()->get_window_s(ev.user.windowID);
        if(w == nullptr){
            return;
        }
        w->handle_draw(ev.user.timestamp);
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
    // For timer in event loop
    struct sdl_timer_event_detail{
        Btk::Atomic refcount = 1;
        bool (*cb)(Uint32 timestamp,void *data1,void *data2) = nullptr;
        SDL_TimerID id = 0;
        void *userdata1 = nullptr;
        void *userdata2 = nullptr;
        bool active = true;

        void unref(){
            --refcount;
            if(refcount == 0){
                delete this;
            }
        }
        void ref(){
            ++refcount;
        }
    };
    Uint32 sdl_timer_event_cb(Uint32 interval,void *param){
        SDL_Event event;
        event.type = Btk::GetSystem()->timer_timeout_ev_id;
        event.user.timestamp = SDL_GetTicks();
        event.user.data1 = param;//< For mark which timer

        static_cast<sdl_timer_event_detail*>(param)->ref();
        return interval;
    }
    void timer_timeout_cb(const SDL_Event &event,void *){
        auto detail = static_cast<sdl_timer_event_detail*>(event.user.data1);
        if(detail->active){
            if(not detail->cb(event.user.timestamp,detail->userdata1,detail->userdata2)){
                //Require close timer
                SDL_RemoveTimer(detail->id);
                detail->unref();
                detail->active = false;
            }
        }
        detail->unref();
    }
    void sdl_timer_stop_internal(sdl_timer_event_detail *p){
        if(p != nullptr){
            if(p->active){
                SDL_RemoveTimer(p->id);
                p->active = false;
                p->unref();
            }
        }
    }
    //ResourceBase
    Btk::Constructable<Btk::BasicResource> resource_base;
    bool resource_inited = false;
    //Cleanup / Init
    void resource_cleanup() noexcept{
        if(resource_inited){
            resource_inited = false;
            resource_base.destroy();
        }
    }
    void resource_init(){
        if(not resource_inited){
            resource_inited = true;
            resource_base.construct();
            std::atexit(resource_cleanup);
        }
    }
    struct _interrupt_loop_t{
        //Interrupt
    };
}

namespace Btk{
    inline BasicResource::BasicResource(){
        //Image Adapter
        InitImageAdapter();
    }
    inline BasicResource::~BasicResource(){
        //run all exit handlers
        for(auto &handler:atexit_handlers){
            handler();
        }
        //unload modules
        for(auto &mod:modules_list){
            mod.unload();
        }
        //Final Quit System
        System::Quit();
    }
    //register a exit handler
    inline
    void BasicResource::atexit(void (*fn)(void *),void *data){
        atexit_handlers.push_back({
            fn,
            data
        });
    }
    //std c handlers
    inline
    void BasicResource::atexit(void (*fn)()){
        this->atexit(callback_wrapper,reinterpret_cast<void*>(fn));
    }
}

//Event translate
namespace{
    //Btk Translate Event
    auto tr_event(const SDL_MouseMotionEvent &event) noexcept -> Btk::MotionEvent{
        Btk::MotionEvent ev;

        ev.x = event.x;
        ev.y = event.y;
        ev.xrel = event.xrel;
        ev.yrel = event.yrel;

        return ev;
    };
    auto tr_event(const SDL_MouseWheelEvent &event) noexcept -> Btk::WheelEvent{
        Sint64 x,y;
        if(event.direction == SDL_MOUSEWHEEL_FLIPPED){
            x = -event.x;
            y = -event.y;
        }
        else{
            x = event.x;
            y = event.y;
        }
        return {event.which,x,y};
    }
    auto tr_event(const SDL_MouseButtonEvent &event) noexcept -> Btk::MouseEvent{
        Btk::MouseEvent ev;

        if(event.state == SDL_PRESSED){
            ev.state = Btk::MouseEvent::Pressed;
        }
        else{
            ev.state = Btk::MouseEvent::Released;
        }
        ev.x = event.x;
        ev.y = event.y;

        ev.clicks = event.clicks;
        ev.button.value = event.button;
        return ev;
    }
    auto tr_event(const SDL_KeyboardEvent &event) noexcept -> Btk::KeyEvent{
        Btk::KeyEvent ev;
        if(event.state == SDL_PRESSED){
            ev.state = Btk::KeyEvent::Pressed;
        }
        else{
            ev.state = Btk::KeyEvent::Released;
        }

        ev.keycode = Btk::Keycode(event.keysym.sym);
        ev.scancode = Btk::Scancode(event.keysym.scancode);
        ev.keymode = static_cast<Btk::Keymode>(event.keysym.mod);

        ev.repeat = event.repeat;
        return ev;
    }
    auto tr_event(const SDL_DropEvent &event) noexcept -> Btk::DropEvent{
        Btk::Event::Type type;
        Btk::DropEvent ev(Btk::Event::None);

        switch(event.type){
            case SDL_DROPBEGIN:
                ev.set_type(Btk::Event::DropBegin);
                BTK_LOGINFO("DropBegin");
                break;
            case SDL_DROPCOMPLETE:
                ev.set_type(Btk::Event::DropEnd);
                BTK_LOGINFO("DropEnd");
                break;
            case SDL_DROPFILE:
                ev.set_type(Btk::Event::DropFile);
                ev.text = event.file;
                BTK_LOGINFO("DropFile");
                break;
            case SDL_DROPTEXT:
                ev.set_type(Btk::Event::DropText);
                ev.text = event.file;
                BTK_LOGINFO("DropText");
                break;
            default:
                BTK_PANIC("Impossible");
        }
        return ev;
    }
    auto tr_event(const SDL_TextEditingEvent &t) noexcept -> Btk::TextEditingEvent{
        Btk::TextEditingEvent event;

        event.text = t.text;
        event.editing = Btk::u8string_view(&t.text[t.start],t.length);
        
        return event;
    }
}

namespace Btk{
    //< The thread id of thread which called Btk::run
    static std::thread::id main_thrd;
    System* System::instance = nullptr;
    bool    System::is_running = false;

    #ifndef NDEBUG
    static int loop_depth = 0;
    //< MessageLoop depth
    #endif

    void Init(){
        resource_init();
        //Init Btk
        System::Init();
    }
    void Quit(){
        resource_cleanup();
    }
    int  run(){
        if(main_thrd == std::thread::id()){
            main_thrd = std::this_thread::get_id();
        }
       
        if(main_thrd != std::this_thread::get_id() or System::is_running == true){
            //double call or call from another thread
            return -1;
        }
        int retvalue = EXIT_SUCCESS;
        //resume from error
        resume:
            GetSystem()->is_running = true;
        try{
            GetSystem()->run();
        }
        catch(int i){
            //Exit
            if(not GetSystem()->on_quit()){
                //Resume
                goto resume;
            }
            GetSystem()->is_running = false;
            retvalue = i;
        }
        catch(std::exception &exp){
            
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException] %s",exp.what());
            if(GetSystem()->try_handle_exception(&exp)){
                goto resume;
            }
            GetSystem()->is_running = false;
            throw;
        }
        catch(...){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"[System::GetException]Unknown");
            if(GetSystem()->try_handle_exception(nullptr)){
                goto resume;
            }
            GetSystem()->is_running = false;
            throw;
        }
        GetSystem()->is_running = false;
        return retvalue;
    }
    auto PollEvent() -> LoopStatus{
        #ifndef NDEBUG
        Btk_defer [](){
            BTK_LOGINFO("[System::Loop]Leaving Loop");
            loop_depth -= 1;
        };
        loop_depth += 1;
        BTK_LOGINFO("[System::Loop]Enter Loop");
        #endif

        try{
            SDL_Event event;
            while(SDL_PollEvent(&event)){
                GetSystem()->dispatch_event(event);
            }
            GetSystem()->on_idle();
            return LoopStatus::Running;
        }
        catch(_interrupt_loop_t){
            return LoopStatus::Interrupted;
        }
        catch(...){
            DeferRethrow();
            return LoopStatus::Exception;
        }
    }
    auto WaitEvent() -> LoopStatus{
        #ifndef NDEBUG
        Btk_defer [](){
            BTK_LOGINFO("[System::Loop]Leaving Loop");
            loop_depth -= 1;
        };
        loop_depth += 1;
        BTK_LOGINFO("[System::Loop]Enter Loop");
        #endif

        try{
            SDL_Event event;
            while(true){
                GetSystem()->wait_event(event);
                GetSystem()->dispatch_event(event);
            }
            return LoopStatus::Running;
        }
        catch(_interrupt_loop_t){
            return LoopStatus::Interrupted;
        }
        catch(...){
            DeferRethrow();
            return LoopStatus::Exception;
        }
    }
    System::System(){
        defer_call_ev_id = SDL_RegisterEvents(3);
        if(defer_call_ev_id == (Uint32)-1){
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"Could not regitser event");
        }
        else{
            redraw_win_ev_id = defer_call_ev_id + 1;
            timer_timeout_ev_id = redraw_win_ev_id + 1;
            //regitser handler
            regiser_eventcb(defer_call_ev_id,defer_call_cb,nullptr);
            regiser_eventcb(redraw_win_ev_id,redraw_win_cb,nullptr);
            regiser_eventcb(timer_timeout_ev_id,timer_timeout_cb,nullptr);
        }

        //First stop text input
        SDL_StopTextInput();
    }
    System::~System(){
        while(not wins_map.empty()){
            close_window(wins_map.begin()->second);
        }
    }
    //Init or Quit
    int  System::Init(){
        if(instance == nullptr){
            if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1){
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
            //Init platform
            GL::Init();
            Font::Init();
            Platform::Init();
            
            //Load module by env
            #ifndef BTK_NO_AUTOLOAD
            const char *mods = SDL_getenv("BTK_MODULES");
            if(mods != nullptr){
                u8string_view view(mods);
                for(auto &name:view.split(';')){
                    LoadModule(name);
                }
            }
            #endif

        }
        return 1;
    }
    //Global Cleanup
    void System::Quit(){
        if(instance != nullptr){
            BTK_ASSERT(GetSystem()->is_running == false);
            //delete instance to cleanup windows
            delete instance;
            instance = nullptr;
            //Cleanup platform
            Platform::Quit();
            //Quit SDL
            // TTF_Quit();
            Font::Quit();
            SDL_Quit();

        }
    }
    //EventLoop
    void System::dispatch_event(SDL_Event &event){
        switch(event.type){
            case SDL_QUIT:{
                BTK_LOGINFO("[System::Core]Got SDL_QUIT");
                Exit(EXIT_SUCCESS);
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
            case SDL_TEXTEDITING:{
                on_textediting(event);
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
            #if BTK_MOBILE
            case SDL_APP_TERMINATING:{
                signal_app_terminating();
                break;
            }
            case SDL_APP_LOWMEMORY:{
                signal_app_lowmemory();
                break;
            }
            #endif

            #ifdef SDL_LOCALECHANGED
            case SDL_LOCALECHANGED:{
                signal_locale_changed();
                break;
            }
            #endif
            
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
    inline void System::run(){
        SDL_Event event;
        while(true){
            wait_event(event);
            dispatch_event(event);
        }
    }
    inline void System::wait_event(SDL_Event &event){
        if(idle_handlers.empty()){
            //Just use sdl wait event
            SDL_WaitEvent(&event);
            return;
        }
        while(SDL_PollEvent(&event) == 0){
            //Wait for event
            on_idle();
            SDL_Delay(wait_event_delay);
        }
    }
    inline void System::on_idle(){
        // BTK_LOGINFO("IDLE");
        for(auto &handler:idle_handlers){
            handler();
        }
    }
    //WindowEvent
    inline void System::on_windowev(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.window.windowID);
        if(win == nullptr){
            return;
        }
        SDLEvent ev(&event);
        win->handle(ev);
    }
    //MouseMotion
    inline void System::on_mousemotion(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.motion.windowID);
        if(win == nullptr){
            return;
        }
        auto motion = tr_event(event.motion);
        win->handle_motion(motion);
    }
    //MouseButton
    inline void System::on_mousebutton(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.button.windowID);
        if(win == nullptr){
            return;
        }
        auto click = tr_event(event.button);
        win->handle_mouse(click);
    }
    //MouseWheel
    inline void System::on_mousewheel(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.button.windowID);
        if(win == nullptr){
            return;
        }
        auto wheel = tr_event(event.wheel);
        if(win->handle_wheel(wheel)){
            if(not wheel.is_accepted()){
                win->signal_event().emit(wheel);
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
        auto drop = tr_event(event.drop);
        win->handle_drop(drop);
    }
    //KeyBoardEvent
    inline void System::on_keyboardev(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.key.windowID);
        if(win == nullptr){
            return;
        }
        auto kevent = tr_event(event.key);
        if(not win->handle_keyboard(kevent)){
            //No one process it
            if(not kevent.is_accepted()){
                win->signal_event().emit(kevent);
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
    inline void System::on_textediting(const SDL_Event &event){
        WindowImpl *win = get_window_s(event.text.windowID);
        if(win == nullptr){
            return;
        }
        auto ev = tr_event(event.edit);
        win->handle_textediting(ev);
    }
    inline bool System::on_quit(){
        if(signal_quit.empty()){
            //Deafult close all the window and quit
            BTK_LOGINFO("[System::Core]Try to quit");
            std::lock_guard locker(map_mtx);
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
                //Do quit
                return true;
            }
            return false;
        }
        else{
            BTK_LOGINFO("[System::Core]Emitting SignalQuit");
            signal_quit();
            return true;
        }
    }
    void System::register_window(WindowImpl *impl){
        if(impl == nullptr){
            return;
        }
        std::lock_guard locker(map_mtx);
        Uint32 winid = SDL_GetWindowID(impl->win);
        //GetWindowId
        wins_map.insert(std::make_pair(winid,impl));
    }
    void System::unregister_window(WindowImpl *impl){
        if(impl == nullptr){
            return;
        }
        std::lock_guard locker(map_mtx);
        Uint32 winid = impl->id();
        
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

            #ifndef NDEBUG
            //Dump still alive window
            BTK_LOGINFO("");//Space
            BTK_LOGINFO("--- Still alive Windows ---");//Space
            for(auto &pair:wins_map){
                BTK_ASSERT(pair.second != nullptr);
                Uint32 flags;
                int x,y;
                int w,h;

                flags = SDL_GetWindowFlags(pair.second->win);
                bool visuable = (flags & SDL_WINDOW_SHOWN) != 0;
                
                SDL_GetWindowPosition(pair.second->win,&x,&y);
                SDL_GetWindowSize(pair.second->win,&w,&h);
                BTK_LOGINFO("[Window %d]'%s' At Desktop(%d,%d,%d,%d) %s",
                    pair.first,
                    SDL_GetWindowTitle(pair.second->win),
                    x,y,w,h,
                    visuable ? "Visiable" : "Invisiable"
                );
            }
            BTK_LOGINFO("--- End Dump ---");
            BTK_LOGINFO("");//Space
            #endif
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
            #ifndef NDEBUG
            //Debug info
            SDL_Window *win = SDL_GetWindowFromID(winid);
            if(win == nullptr){
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,"[SDL2]:%u Invalid SDLWindowID",winid);
            }
            else{
                int w,h;
                int x,y;
                SDL_GetWindowPosition(win,&x,&y);
                SDL_GetWindowSize(win,&w,&h);
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
                    "[SDL2]%u => %p title '%s' position (%d,%d) size (%d,%d)",
                    winid,
                    win,
                    SDL_GetWindowTitle(win),
                    x,y,
                    w,h
                );
            }
            #endif
            return nullptr;
        }
        else{
            return iter->second;
        }
    }
    WindowImpl *System::get_window_s(Uint32 winid){
        std::lock_guard locker(map_mtx);
        return get_window(winid);
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
            win->_signal_closed.nothrow_emit();
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

        #ifndef NDEBUG
        int display = SDL_GetWindowDisplayIndex(win);
        float vdpi,hdpi;
        SDL_GetDisplayDPI(display,nullptr,&hdpi,&vdpi);
        BTK_LOGINFO("[Window::DPI]win v : %f h: %f",vdpi,hdpi);
        #endif
        return p;

    }
}
namespace Btk{
    //AsyncSystem
    inline
    AsyncSystem::AsyncSystem(){
        //Create a new worker
        create_worker();
    }
    inline
    AsyncSystem::~AsyncSystem(){
        for(auto iter = workers.begin();iter != workers.end();){
            //Send a quit command
            iter->ask_quit();
            iter = workers.erase(iter);
        }
    }
    inline
    void AsyncSystem::create_worker(){
        workers.emplace_back();
    }
    inline
    void AsyncSystem::add_task(const Task &task){
        for(auto &worker:workers){
            if(worker.idle){
                worker.add_task(task);
                return;
            }
        }
        if(workers.size() < workerd_limit){
            create_worker();
            workers.back().add_task(task);
            return;
        }
        //Find the worker with least tasks
        auto iter = std::min_element(workers.begin(),workers.end(),
            [](const Worker &lhs,const Worker &rhs){
                return lhs.tasks_queue.size() < rhs.tasks_queue.size();
            }
        );
        iter->add_task(task);
    }

    using _AsyncWorker = AsyncSystem::Worker;
    using _AsyncTask = AsyncSystem::Task;
    void _AsyncWorker::run(){
        BTK_LOGINFO("[AsyncWorker]Init %p",this);
        Task task;
        while(true){
            queue_lock.lock();
            if(tasks_queue.empty()){
                idle = true;
                queue_lock.unlock();
                event.clear();
                event.wait();
                continue;
            }
            task = tasks_queue.front();
            tasks_queue.pop();
            queue_lock.unlock();
            try{
                task();
            }
            catch(int){
                //Ask return the thread
                BTK_LOGINFO("[AsyncWorker]Quit %p",this);
                return;
            }
            catch(...){
                DeferRethrow();
            }
        }
    }
    inline
    void _AsyncWorker::ask_quit(){
        Task task;
        task.data = nullptr;
        task.fn = [](void *){
            throw 1;
        };
        std::lock_guard locker(queue_lock);
        tasks_queue.emplace(task);
        event.set();
    }
    inline
    void _AsyncWorker::add_task(const Task &task){
        std::lock_guard locker(queue_lock);
        tasks_queue.emplace(task);
        event.set();
    }
    void AsyncCall(void (*fn)(void*),void *data){
        //Check is inited
        Init();
        AsyncSystem::Task task;
        task.data = data;
        task.fn = fn;
        GetSystem()->async.add_task(task);
    }
}
namespace Btk{
    static void exit_impl_cb(void *args){
        int v = LoadPodInPointer<int>(args);
        DeletePodInPointer<int>(args);
        throw v;
    }
    static void interrupt_impl_cb(){
        throw _interrupt_loop_t{};
    }
    void Exit(int code){
        void *args = nullptr;
        NewPodInPointer<int>(&args,code);
        DeferCall(exit_impl_cb,args);
    }
    void InterruptLoop(){
        DeferCall(interrupt_impl_cb);
    }
    ExceptionHandler SetExceptionHandler(ExceptionHandler handler){
        auto current = GetSystem()->handle_exception;
        GetSystem()->handle_exception = handler;
        return current;
    }
    void AtExit(void(* fn)(void*),void *data){
        resource_init();
        resource_base->atexit(fn,data);
    }
    void AtExit(void(* fn)()){
        resource_init();
        resource_base->atexit(fn);
    }
    void AtIdle(void(*fn)(void*),void *data){
        GetSystem()->idle_handlers.push_back({fn,data});
    }
    void AtIdle(void(*fn)()){
        AtIdle(callback_wrapper,reinterpret_cast<void*>(fn));
    }

    void DeferCall(void(*fn)(void*),void *userdata){
        BTK_ASSERT(GetSystem() != nullptr);
        
        GetSystem()->defer_call(
            fn,userdata
        );
    }
    void DeferCall(void(*fn)()){
        GetSystem()->defer_call(
            callback_wrapper,reinterpret_cast<void*>(fn)
        );
    }
    void DeferRethrow(){
        DeferCall(std::rethrow_exception,std::current_exception());
    }
    bool IsMainThread(){
        return main_thrd == std::this_thread::get_id();
    }
    bool CouldBlock(){
        return not GetSystem()->is_running or not IsMainThread();
    }
    void RegisterImageAdapter(const ImageAdapter & a){
        resource_init();
        resource_base->image_adapters.push_front(a);
        BTK_LOGINFO("[System::Resource]Register ImageAdapter %s",a.name);
    }
    void RegisterDevice(System::CreateDeviceFn fn){
        resource_init();
        if(fn == nullptr){
            return;
        }
        resource_base->devices_list.emplace_front(fn);
    }
    SDL_Surface *LoadImage(SDL_RWops *rwops,u8string_view type){
        resource_init();
        BTK_ASSERT(rwops != nullptr);
        SDL_Surface *ret;
        if(type.empty()){
            //Didnot provide the type
            for(auto &adapter:resource_base->image_adapters){
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
        for(auto &adapter:resource_base->image_adapters){
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
    void SaveImage(SDL_RWops *rw,SDL_Surface *surf,u8string_view type,int quality){
        resource_init();
        quality = std::clamp(quality,0,10);
        if(type.empty()){
            throwRuntimeError("empty type");
        }
        for(auto &adapter:resource_base->image_adapters){
            if(SDL_strncasecmp(adapter.name,type.data(),type.size()) == 0){
                //Got Adapter
                if(adapter.fn_save == nullptr){
                    //Ignore it
                    continue;
                }
                else if(SDL_strncasecmp(adapter.name,type.data(),type.size()) != 0){
                    //Is not the type we wanted 
                    continue;
                }
                if(adapter.save(rw,surf,quality)){
                    //OK
                    return;
                }
                //Failed
                throwRuntimeError(SDL_GetError());
            }
        }
        throwRuntimeError("Unsupport format");
    }
    ImageDecoder *CreateImageDecoder(u8string_view type,u8string_view vendor){
        resource_init();
        if(type.empty()){
            return nullptr;
        }
        for(auto &adapter:resource_base->image_adapters){
            if(adapter.create_decoder == nullptr){
                continue;
            }
            if(SDL_strncasecmp(adapter.name,type.data(),type.length()) == 0){
                //Same type
                if(not vendor.empty()){
                    //Check vendor
                    if(SDL_strncasecmp(adapter.vendor,vendor.data(),vendor.length()) != 0){
                        //Failed
                        continue;
                    }
                }
                return adapter.create_decoder();
            }
        }
        throwRuntimeError("Unsupport type");
    }
    ImageDecoder *CreateImageDecoder(SDL_RWops *rwops,bool autoclose){
        resource_init();
        for(auto &adapter:resource_base->image_adapters){
            if(adapter.create_decoder == nullptr){
                continue;
            }
            if(adapter.fn_is != nullptr){
                //Same type
                if(adapter.is(rwops)){
                    std::unique_ptr<ImageDecoder> decoder(
                        adapter.create_decoder()
                    );
                    decoder->open(rwops,autoclose);
                    return decoder.release();
                }
            }
        }
        throwRuntimeError("Unsupport type");
    }
    RendererDevice *CreateDevice(SDL_Window *win){
        //It will crash at here if we put it into resource_base->create_device(win);
        //and enable optimition in gcc
        resource_init();
        RendererDevice *dev;

        #ifdef BTK_HAVE_SOFTWARE_DEVICE
        try{
        #endif
        
        for(auto fn:resource_base->devices_list){
            dev = fn(win);
            if(dev != nullptr){
                return dev;
            }
        }
        #ifdef BTK_HAVE_SOFTWARE_DEVICE
        }catch(RuntimeError &){

        }
        return new Btk::SWDevice(win);
        #endif
        return nullptr;
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
        resource_init();
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

        resource_base->modules_list.push_back(mod);
    }
    bool HasModule(u8string_view name){
        resource_init();
        for(auto &mod:resource_base->modules_list){
            if(mod.name == name){
                return true;
            }
        }
        return false;
    }
}

BTK_CDECLS_BEGIN

#ifndef NDEBUG
static bool nolog = false;

Btk_CallOnLoad{
    nolog = std::getenv("BTK_NOLOG");
};

#endif

void _BtkL_Info(const char *fmt, ...){
    #ifndef NDEBUG
    if(nolog){
        return;
    }
    if(Btk::loop_depth > 0){
        std::va_list varg;
        va_start(varg,fmt);
        auto s = Btk::u8vformat(fmt,varg);
        va_end(varg);
        //add space by loop depth
        for(int i = 0;i < Btk::loop_depth;i++){
            s.prepend("    ");
        }
        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,"%s",s.c_str());
        return;
    }
    #endif
    std::va_list varg;
    va_start(varg,fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_INFO,fmt,varg);
    va_end(varg);
}
void _BtkL_Warn(const char *fmt, ...){
    #ifndef NDEBUG
    if(nolog){
        return;
    }
    if(Btk::loop_depth > 0){
        std::va_list varg;
        va_start(varg,fmt);
        auto s = Btk::u8vformat(fmt,varg);
        va_end(varg);
        //add space by loop depth
        for(int i = 0;i < Btk::loop_depth;i++){
            s.prepend("    ");
        }
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,"%s",s.c_str());
        return;
    }
    #endif
    std::va_list varg;
    va_start(varg,fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_WARN,fmt,varg);
    va_end(varg);
}
void _BtkL_Error(const char *fmt, ...){
    #ifndef NDEBUG
    if(nolog){
        return;
    }
    if(Btk::loop_depth > 0){
        std::va_list varg;
        va_start(varg,fmt);
        auto s = Btk::u8vformat(fmt,varg);
        va_end(varg);
        //add space by loop depth
        for(int i = 0;i < Btk::loop_depth;i++){
            s.prepend("    ");
        }
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,"%s",s.c_str());
        return;
    }
    #endif
    std::va_list varg;
    va_start(varg,fmt);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION,SDL_LOG_PRIORITY_ERROR,fmt,varg);
    va_end(varg);
}

BTK_CDECLS_END