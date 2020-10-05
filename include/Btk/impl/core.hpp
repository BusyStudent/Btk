#if !defined(_BOXIMPL_CORE_HPP_)
#define _BOXIMPL_CORE_HPP_
#include <SDL2/SDL.h>
#include <exception>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <list>
namespace Btk{
    struct WindowImpl;
    struct System{
        System();
        ~System();
        static System *instance;//instance of system
        static bool    is_running;
        void run();
        //Window Register
        void register_window(WindowImpl *impl);
        void unregister_window(WindowImpl *impl);
        //Handle event
        void on_windowev(SDL_Event &event);//Handle SDL_WINDOWEVENT
        void on_dropev(SDL_Event &event);//Handle SDL_DropEvent
        void on_mousemotion(SDL_Event &event);//Handle SDL_MouseMotion
        //defercall in eventloop
        void defer_call(void(* fn)(void*),void *data = nullptr);
        //Get window from WindowID
        WindowImpl *get_window(Uint32 winid);

        std::unordered_map<Uint32,WindowImpl*> wins_map;//Windows map
        std::unordered_map<Uint32,std::function<void(SDL_Event&)>> evcbs_map;//Event callbacks map
        std::recursive_mutex map_mtx;
        Uint32 defer_call_ev_id;//defer call Event ID
        //called after a exception was throwed
        //return false to abort program
        bool (*handle_exception)(std::exception *);
        //called atexit
        std::list<std::function<void()>> atexit_handlers;
        void atexit(std::function<void()> &&func);
    };
    extern int  Main();
    extern void Init();
    //Exit the app
    extern void Exit(int code);
};


#endif // _BOXIMPL_CORE_HPP_
