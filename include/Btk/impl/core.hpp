#if !defined(_BOXIMPL_CORE_HPP_)
#define _BOXIMPL_CORE_HPP_
#include <SDL2/SDL.h>
#include <unordered_map>
#include <mutex>
namespace Btk{
    struct WindowImpl;
    struct System{
        System();
        ~System();
        static System *current;
        void run();
        void register_window(WindowImpl *impl);
        
        std::unordered_map<Uint32,WindowImpl*> wins_map;
        std::mutex map_mtx;
        
    };
    extern int  Main();
    extern void Init();
};


#endif // _BOXIMPL_CORE_HPP_
