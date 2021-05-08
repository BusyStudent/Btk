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
namespace{
    //internal function to push event
    using Btk::Event;
    using Btk::Window;
    using Btk::Widget;
    using Btk::System;
    using Btk::WindowImpl;
    void internal_pushevent(Event *ev,WindowImpl *win,Widget *widget){
        Uint32 winid = win->id();
        SDL_Event sdl_ev;
        SDL_zero(sdl_ev);

        sdl_ev.type = Btk::Instance().dispatch_ev_id;
        sdl_ev.user.timestamp = SDL_GetTicks();
        sdl_ev.user.windowID = winid;
        sdl_ev.user.data1 = ev;
        sdl_ev.user.data2 = widget;

        if(SDL_PushEvent(&sdl_ev) != 1){
            //Queue is full or Something error happened
            //delete it
            delete ev;
        }
    }
};
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
    //PushEvent in event queue
    void PushEvent(Event *ev,Window &receiver){
        if(ev != nullptr){
            internal_pushevent(ev,receiver.impl(),nullptr);
        }
    }
    void PushEvent(Event *ev,Widget &receiver){
        if(ev != nullptr){
            internal_pushevent(ev,receiver.window(),&receiver);
        }
    }
    //Send event directly
    bool SendEvent(Event &ev,Window &receiver){
        return receiver.impl()->dispatch(ev);
    }
    bool SendEvent(Event &ev,Widget &receiver){
        return receiver.handle(ev);
    }
    //Dispatch our event to window or widgets
    void DispatchEvent(const SDL_Event &ev,void*){
        WindowImpl *win = Instance().get_window_s(ev.user.windowID);
        if(win == nullptr){
            //Window is not exists
            //ignore it
            return;
        }
        Event *event = static_cast<Event*>(ev.user.data1);
        Widget *widget = static_cast<Widget*>(ev.user.data2);
        std::unique_ptr<Event> ptr(event);
        if(widget == nullptr){
            //Dispatch it on windows
            win->dispatch(*event);
        }
        else{
            //Dispatch it on Widgets
            if(not widget->handle(*event)){
                //It doesnnot accept it
                if(not event->is_accepted()){
                    if(not win->sig_event.empty()){
                        win->sig_event(*event);
                    }
                }
            }
        }
    }
};
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

        ev.keycode = Keycode(event.keysym.sym);
        ev.scancode = Scancode(event.keysym.scancode);
        ev.keymode = static_cast<Keymode>(event.keysym.mod);

        ev.repeat = event.repeat;
        return ev;
    }
};