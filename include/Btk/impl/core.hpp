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
        struct ExitHandler{
            //Function pointer
            typedef void (*FnPtr)(void*);
            FnPtr fn;
            void *data;
            void operator ()() const{
                fn(data);
            }
        };
        struct EventHandler{
            typedef void (*FnPtr)(const SDL_Event &ev,void *data);
            FnPtr fn;
            void *data;
            void operator()(SDL_Event &ev) const{
                fn(ev,data);
            }
        };
        System();
        ~System();
        static System *instance;//instance of system
        static bool    is_running;
        void run();
        //Window Register
        void register_window(WindowImpl *impl);
        void unregister_window(WindowImpl *impl);
        //Handle event
        inline void on_dropev(const SDL_Event &event);//Handle SDL_DropEvent
        inline void on_windowev(const SDL_Event &event);//Handle SDL_WINDOWEVENT
        inline void on_keyboardev(const SDL_Event &event);//Handle SDL_KeyboardEvent
        inline void on_mousemotion(const SDL_Event &event);//Handle SDL_MouseMotion
        inline void on_mousebutton(const SDL_Event &event);//Handle SDL_MouseButton
        //defercall in eventloop
        void defer_call(void(* fn)(void*),void *data = nullptr);
        //Get window from WindowID
        WindowImpl *get_window(Uint32 winid);

        std::unordered_map<Uint32,WindowImpl*> wins_map;//Windows map
        std::unordered_map<Uint32,EventHandler> evcbs_map;//Event callbacks map
        std::recursive_mutex map_mtx;
        Uint32 defer_call_ev_id;//defer call Event ID
        Uint32 dispatch_ev_id;//dispatch our event Event ID
        //called after a exception was throwed
        //return false to abort program
        bool (*handle_exception)(std::exception *);
        //called atexit
        std::list<ExitHandler> atexit_handlers;
        //register handlers
        void atexit(void (*fn)(void *),void *data);
        void atexit(void (*fn)());

        void regiser_eventcb(Uint32 evid,EventHandler::FnPtr ptr,void *data);
        //Init Global
        static int  Init();
        static void Quit();
    };
    extern int  run();
    extern void Init();
    //Exit the app
    extern void Exit(int code);
};


#endif // _BOXIMPL_CORE_HPP_
