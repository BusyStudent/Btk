#if !defined(_BOXIMPL_CORE_HPP_)
#define _BOXIMPL_CORE_HPP_
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_events.h>
#include <condition_variable>
#include <unordered_map>
#include <exception>
#include <atomic>
#include <thread>
#include <mutex>
#include <queue>
#include <list>

#include "atomic.hpp"
#include "thread.hpp"
#include "codec.hpp"
#include "../module.hpp"
#include "../string.hpp"
#include "../object.hpp"


namespace Btk{
    class WindowImpl;
    class RendererDevice;
    struct GenericHandler{
        //Function pointer
        typedef void (*FnPtr)(void*);
        FnPtr fn;
        void *data;
        void operator ()() const{
            fn(data);
        }
    };
    using ExitHandler = GenericHandler;
    typedef RendererDevice *(*CreateDeviceFn)(SDL_Window*);
    /**
     * @brief Generic Resource
     * 
     */
    struct BTKHIDDEN BasicResource{
        inline BasicResource();
        inline ~BasicResource();
        /**
         * @brief Create a device object
         * 
         * @return Device* 
         */
        inline
        RendererDevice *create_device(SDL_Window*);
        //register handlers
        inline
        void atexit(void (*fn)(void *),void *data);
        inline
        void atexit(void (*fn)());

        std::list<ExitHandler> atexit_handlers;
        std::list<ImageAdapter> image_adapters;
        std::list<CreateDeviceFn> devices_list;
        std::list<Module> modules_list;
    };
    struct BTKHIDDEN AsyncSystem{
        //Async Pool
        inline
        AsyncSystem();
        inline
        ~AsyncSystem();

        using Task = GenericHandler;

        struct Worker{
            inline
            Worker():
                thread("BtkWorker",&Worker::run,this){
                
            };
            Worker(const Worker &) = delete;
            ~Worker() = default;

            Atomic idle = false;
            SpinLock queue_lock;
            std::queue<Task> tasks_queue;
            SyncEvent event;
            Thread thread;
            
            inline
            void run();

            inline
            void ask_quit();

            inline
            void add_task(const Task &);
        };
        inline
        void create_worker();
        inline
        void add_task(const Task &task);

        int workerd_limit = std::thread::hardware_concurrency();
        std::list<Worker> workers;
    };
    /**
     * @brief Btk System
     * 
     */
    struct BTKHIDDEN System{
        struct EventHandler{
            typedef void (*FnPtr)(const SDL_Event &ev,void *data);
            FnPtr fn;
            void *data;
            void operator()(SDL_Event &ev) const{
                fn(ev,data);
            }
        };
        using Device = RendererDevice;
        typedef Device *(*CreateDeviceFn)(SDL_Window*);
        
        System();
        System(const System &) = delete;
        ~System();
        
        static System *instance;//instance of system
        static bool    is_running;
        
        //Window Register
        void register_window(WindowImpl *impl);
        void unregister_window(WindowImpl *impl);
        void close_window(WindowImpl *impl);
        //Handle event
        inline void run();
        /**
         * @brief Wait for SDL_Event
         * 
         * @param event 
         * @return int 
         */
        inline void wait_event(SDL_Event &event);
        inline void dispatch_event(SDL_Event &event);
        inline void on_idle();//< When there is no event avliable
        inline void on_dropev(const SDL_Event &event);//Handle SDL_DropEvent
        inline void on_windowev(const SDL_Event &event);//Handle SDL_WINDOWEVENT
        inline void on_keyboardev(const SDL_Event &event);//Handle SDL_KeyboardEvent
        inline void on_mousewheel(const SDL_Event &event);//Handle SDL_MouseWheel
        inline void on_mousemotion(const SDL_Event &event);//Handle SDL_MouseMotion
        inline void on_mousebutton(const SDL_Event &event);//Handle SDL_MouseButton
        inline void on_textinput(const SDL_Event &event);//Handle SDL_TextInputEvent
        inline void on_textediting(const SDL_Event &event);//Handle SDL_TextInputEvent
        inline void on_quit();//Handle SDL_Quit
        //defercall in eventloop
        void defer_call(void(* fn)(void*),void *data = nullptr);
        //Get window from WindowID
        WindowImpl *get_window(Uint32 winid);//< It is thread unsafe
        WindowImpl *get_window_s(Uint32 winid);//< It is thread safe
        WindowImpl *create_window(SDL_Window *win);

        void regiser_eventcb(Uint32 evid,EventHandler::FnPtr ptr,void *data = nullptr);
        bool try_handle_exception(std::exception *exp);

        std::unordered_map<Uint32,WindowImpl*> wins_map;//Windows map
        std::unordered_map<Uint32,EventHandler> evcbs_map;//Event callbacks map
        // std::recursive_mutex map_mtx;
        SpinLock map_mtx;
        
        Uint32 defer_call_ev_id;//defer call Event ID
        Uint32 redraw_win_ev_id;//redraw window Event ID
        Uint32 timer_timeout_ev_id;//timer timeout Event ID
        Uint32 wait_event_delay = 1;//<Delay in wait event
        //called after a exception was throwed
        //return false to abort program
        bool (*handle_exception)(std::exception *) = nullptr;
        //called atexit
        std::list<GenericHandler> idle_handlers;//< Called on idle
        //<The signal will be init before the window will be removed
        Signal<void(WindowImpl *)> signal_window_closed;
        Signal<void(WindowImpl *)> signal_window_created;
        Signal<void()> signal_clipboard_update;
        Signal<void()> signal_audio_device_added;
        Signal<void()> signal_audio_device_removed;
        Signal<void()> signal_keymap_changed;
        Signal<void()> signal_quit;
        #if BTK_MOBILE
        //<The signals on android and iphone
        Signal<void()> signal_app_terminating;
        Signal<void()> signal_app_lowmemory;
        Signal<void()> signal_app_will_enter_background;
        Signal<void()> signal_app_enter_background;
        Signal<void()> signal_app_leave_background;
        #endif

        AsyncSystem async;
        //Init Global
        static int  Init();
        static void Quit();
    };
    //Iconv functions
    class  _iconv;
    using iconv_t = _iconv*;
    extern iconv_t (*iconv_open)(const char *tocode, const char *fromcode);
    extern int     (*iconv_close)(iconv_t );
    extern size_t  (*iconv)(iconv_t ,const char **,size_t*,char **,size_t*);

    BTKAPI int  run();
    BTKAPI void Init();
    //Exit the app
    BTKAPI void Exit(int code);
    
    [[deprecated("use Btk::GetSystem() instead")]]
    inline System &Instance(){
        return *(System::instance);
    }
    inline System *GetSystem(){
        return System::instance;
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
     * @brief Save a image
     * 
     * @param rw The Ouput Rwops
     * @param surf The target surface
     * @param type The image type
     * @param qulity Compress quality(0 - 10) default(0)
     * @return BTKAPI 
     */
    BTKAPI void         SaveImage(SDL_RWops *rw,SDL_Surface *surf,u8string_view type,int quality = 0);
    /**
     * @brief Register your device
     * 
     * @param fn 
     * @return BTKAPI 
     */
    BTKAPI void RegisterDevice(System::CreateDeviceFn fn);
    /**
     * @brief Load builtin image adapter
     * 
     * @return BTKHIDDEN 
     */
    BTKHIDDEN void InitImageAdapter();
    /**
     * @brief Create a Device object
     * 
     * @param window 
     * @return BTKAPI* 
     */
    BTKAPI RendererDevice *CreateDevice(SDL_Window *window);
    BTKAPI WindowImpl     *GetWindowFromID(Uint32 windowid);
};


#endif // _BOXIMPL_CORE_HPP_
