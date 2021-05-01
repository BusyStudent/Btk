#if !defined(_BOXIMPL_CORE_HPP_)
#define _BOXIMPL_CORE_HPP_
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <condition_variable>
#include <unordered_map>
#include <exception>
#include <atomic>
#include <mutex>
#include <queue>
#include <list>

#include "thread.hpp"
#include "../module.hpp"

namespace Btk{
    class WindowImpl;
    /**
     * @brief Image Adpater for Laoding or Saving Image
     * 
     */
    struct ImageAdapter{
        /**
         * @brief Load the image
         * @param rwops
         * @return nullptr on failure
         */
        SDL_Surface *(*fn_load)(SDL_RWops *);
        /**
         * @brief Save the image
         * 
         * @param rwops
         * @param surf 
         * @param quality
         * @return true 
         * @return false 
         */
        bool (*fn_save)(SDL_RWops *,SDL_Surface *surf,int quality);
        /**
         * @brief Is the image
         * 
         * @return true 
         * @return false 
         */
        bool (*fn_is)(SDL_RWops *);
        const char *name;
    };
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
        inline void on_mousewheel(const SDL_Event &event);//Handle SDL_MouseWheel
        inline void on_mousemotion(const SDL_Event &event);//Handle SDL_MouseMotion
        inline void on_mousebutton(const SDL_Event &event);//Handle SDL_MouseButton
        inline void on_textinput(const SDL_Event &event);
        //defercall in eventloop
        void defer_call(void(* fn)(void*),void *data = nullptr);
        //Get window from WindowID
        WindowImpl *get_window(Uint32 winid);//< It is thread unsafe
        WindowImpl *get_window_s(Uint32 winid);//< It is thread safe
    
        std::unordered_map<Uint32,WindowImpl*> wins_map;//Windows map
        std::unordered_map<Uint32,EventHandler> evcbs_map;//Event callbacks map
        std::recursive_mutex map_mtx;
        Uint32 defer_call_ev_id;//defer call Event ID
        Uint32 dispatch_ev_id;//dispatch our event Event ID
        //called after a exception was throwed
        //return false to abort program
        bool (*handle_exception)(std::exception *) = nullptr;
        //called atexit
        std::list<ExitHandler> atexit_handlers;
        std::list<ImageAdapter> image_adapters;
        //register handlers
        void atexit(void (*fn)(void *),void *data);
        void atexit(void (*fn)());

        void regiser_eventcb(Uint32 evid,EventHandler::FnPtr ptr,void *data = nullptr);

        std::list<Module> modules_list;
        //std::list<RendererCreateFn> render_list;
        //Init Global
        static int  Init();
        static void Quit();
    };
    BTKAPI int  run();
    BTKAPI void Init();
    //Exit the app
    BTKAPI void Exit(int code);
    inline System &Instance(){
        return *(System::instance);
    }
};


#endif // _BOXIMPL_CORE_HPP_
