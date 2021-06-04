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
#include "../string.hpp"
#include "../object.hpp"

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

        SDL_Surface *load(SDL_RWops *rw) const{
            if(fn_load != nullptr){
                return fn_load(rw);
            }
            return nullptr;
        }
        bool is(SDL_RWops *rwops){
            if(fn_is != nullptr){
                return fn_is(rwops);
            }
            return false;
        }
        const char *name;
    };
    struct BTKHIDDEN System{
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
        void close_window(WindowImpl *impl);
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
        WindowImpl *create_window(SDL_Window *win);
        //register handlers
        void atexit(void (*fn)(void *),void *data);
        void atexit(void (*fn)());

        void regiser_eventcb(Uint32 evid,EventHandler::FnPtr ptr,void *data = nullptr);
        bool try_handle_exception(std::exception *exp);

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

        std::list<Module> modules_list;
        //<The signal will be init before the window will be removed
        Signal<void(WindowImpl *)> signal_close_window;
        Signal<void(WindowImpl *)> signal_create_window;
        Signal<void()> signal_clipboard_update;
        Signal<void()> signal_quit;
        //std::list<RendererCreateFn> render_list;
        //Init Global
        static int  Init();
        static void Quit();
    };
    BTKAPI int  run();
    BTKAPI void Init();
    //Exit the app
    BTKAPI void Exit(int code);
    BTKAPI void RegisterImageAdapter(const ImageAdapter &);
    inline System &Instance(){
        return *(System::instance);
    }
    /**
     * @brief Load a image
     * 
     * @param rwops 
     * @param type The image type(could be empty)
     * @return BTKAPI* 
     */
    BTKAPI SDL_Surface *LoadImage(SDL_RWops *rwops,u8string_view type = {});
    /**
     * @brief Load builtin image adapter
     * 
     * @return BTKHIDDEN 
     */
    BTKHIDDEN void InitImageAdapter();
};


#endif // _BOXIMPL_CORE_HPP_
