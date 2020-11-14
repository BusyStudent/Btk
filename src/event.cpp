#include <SDL2/SDL.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
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
    void internal_pushevent(Event *ev,Window &win,Widget *widget){
        Uint32 winid = SDL_GetWindowID(win.impl()->win);
        SDL_Event sdl_ev;
        SDL_zero(sdl_ev);

        sdl_ev.type = System::instance->dispatch_ev_id;
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
    static std::atomic<Event::Type> current_type = Event::USER;
    Event::~Event(){

    }
    //Register a event
    Event::Type Event::Register(){
        auto now = current_type.load(
            std::memory_order::memory_order_consume
        );

        if(now == Type::USER_MAX){
            return Type::ERROR;
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
            internal_pushevent(ev,receiver,nullptr);
        }
    }
    void PushEvent(Event *ev,Widget &receiver){
        if(ev != nullptr){
            internal_pushevent(ev,receiver.master(),&receiver);
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
        WindowImpl *win;
        {
            //trying to get window
            std::lock_guard<std::recursive_mutex> locker(
                System::instance->map_mtx
            );
            win = System::instance->get_window(ev.user.windowID);
        }
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
            widget->handle(*event);
        }
    }
};