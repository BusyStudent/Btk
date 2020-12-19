//Widget impl
#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    Widget::~Widget(){
        
    }
    bool Widget::handle(Event& ev){
        //Default Process event
        if(ev.type() == Event::Type::SetRect){
            rect = event_cast<SetRectEvent&>(ev).rect;
            return true;
        }
        return false;
    }
    void Widget::set_rect(const Rect &rect){
        attr.user_rect = true;
        SetRectEvent ev(rect);
        handle(ev);
    }
    Container::Container(){
        attr.container = true;
    }
    //delete container
    Container::~Container(){
        for(auto iter = widgets_list.begin();iter != widgets_list.end();){
            delete *iter;
            iter = widgets_list.erase(iter);
        }
    }
};
namespace Btk{
    bool Container::handle(Event &event){
        switch(event.type()){
            case Event::Motion:
            case Event::Click:{

            }
            default:{
                //dispatch 
                for(auto w:widgets_list){
                    if(w->handle(event)){
                        if(event.is_accepted()){
                            return true;
                        }
                    }
                }
                return false;
            }
        }
    }
}