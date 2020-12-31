#if !defined(_BTK_LOCALS_HPP_)
#define _BTK_LOCALS_HPP_
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_mouse.h>
//A headers to simply use SDL enums
namespace Btk{
    typedef SDL_Keycode Keycode;
    typedef SDL_Scancode Scancode;
    //Keymode enum
    enum class Keymode:Uint16{
        Ctrl = KMOD_CTRL,
        Shift = KMOD_SHIFT,
        //etc...
    };
    inline Keymode operator |(Keymode a,Keymode b){
        return static_cast<Keymode>
            (static_cast<Uint16>(a) | static_cast<Uint16>(b)
        );
    };
    inline Keymode operator &(Keymode a,Keymode b){
        return static_cast<Keymode>
            (static_cast<Uint16>(a) & static_cast<Uint16>(b)
        );
    };
    inline bool operator ==(Keymode a,Uint16 b){
        return Uint16(a) == b;
    };
    inline bool operator !=(Keymode a,Uint16 b){
        return Uint16(a) != b;
    };
};

#endif // _BTK_LOCALS_HPP_
