#include <SDL2/SDL.h>

#include "build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
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
            std::memory_order::memory_order_consume
        );

        if(now == Type::UserMax){
            return Type::User;
        }
        else{
            current_type.store(
                static_cast<Event::Type>(now + 1),
                std::memory_order_relaxed
            );
        }
        return current_type;
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
    
    UpdateEvent::~UpdateEvent(){}
    KeyEvent::~KeyEvent(){}
    MouseEvent::~MouseEvent(){}
    DragEvent::~DragEvent(){}
    MotionEvent::~MotionEvent(){}
    WheelEvent::~WheelEvent(){}
    size_t TextInputEvent::length() const noexcept{
        return SDL_utf8strlen(text.data());
    }
};
namespace Btk{
    //Btk Translate Event
    MotionEvent TranslateEvent(const SDL_MouseMotionEvent &event){
        MotionEvent ev;

        ev.x = event.x;
        ev.y = event.y;
        ev.xrel = event.xrel;
        ev.yrel = event.yrel;

        return ev;
    };
    WheelEvent TranslateEvent(const SDL_MouseWheelEvent &event){
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
    MouseEvent TranslateEvent(const SDL_MouseButtonEvent &event){
        MouseEvent ev;

        if(event.state == SDL_PRESSED){
            ev.state = MouseEvent::Pressed;
        }
        else{
            ev.state = MouseEvent::Released;
        }
        ev.x = event.x;
        ev.y = event.y;

        ev.clicks = event.clicks;
        ev.button.value = event.button;
        return ev;
    }
    KeyEvent TranslateEvent(const SDL_KeyboardEvent &event){
        KeyEvent ev;
        if(event.state == SDL_PRESSED){
            ev.state = KeyEvent::Pressed;
        }
        else{
            ev.state = KeyEvent::Released;
        }

        ev.keycode = event.keysym.sym;
        ev.scancode = event.keysym.scancode;
        ev.keymode = static_cast<Keymode>(event.keysym.mod);

        ev.repeat = event.repeat;
        return ev;
    }
    DropEvent TranslateEvent(const SDL_DropEvent &event){
        Event::Type type;
        DropEvent ev(Event::None);

        switch(event.type){
            case SDL_DROPBEGIN:
                ev.set_type(Event::DropBegin);
                BTK_LOGINFO("DropBegin");
                break;
            case SDL_DROPCOMPLETE:
                ev.set_type(Event::DropEnd);
                BTK_LOGINFO("DropEnd");
                break;
            case SDL_DROPFILE:
                ev.set_type(Event::DropFile);
                ev.text = event.file;
                BTK_LOGINFO("DropFile");
                break;
            case SDL_DROPTEXT:
                ev.set_type(Event::DropText);
                ev.text = event.file;
                BTK_LOGINFO("DropText");
                break;
            default:
                BTK_ASSERT(!"");
        }
        return ev;
    }
};