#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/detail/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>
#include <memory>
#include <atomic>
#include <mutex>
namespace Btk{
    static std::atomic<Event::Type> current_type = Event::User;
    //Register a event
    Event::Type Event::Register(){
        auto now = current_type.load(
            std::memory_order_consume
        );

        if(now == Type::UserMax){
            return Type::Error;
        }
        else{
            return current_type.exchange(
                static_cast<Event::Type>(now + 1),
                std::memory_order_relaxed
            );
        }
    }

    //Send event directly
    bool SendEvent(Event &ev,Window &receiver){
        return receiver.impl()->handle(ev);
    }
    bool SendEvent(Event &ev,Widget &receiver){
        return receiver.handle(ev);
    }
}
namespace Btk{
    //Another Event
    
    size_t TextInputEvent::length() const noexcept{
        return SDL_utf8strlen(text.data());
    }
};
namespace Btk{
    std::ostream &operator <<(std::ostream &ostream,Keycode code){
        ostream << SDL_GetKeyName(SDL_Keycode(code));
        return ostream;
    }
    std::ostream &operator <<(std::ostream &ostream,Scancode code){
        ostream << SDL_GetScancodeName(SDL_Scancode(code));
        return ostream;
    }
}