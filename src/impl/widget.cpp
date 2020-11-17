//Widget impl
#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>
namespace Btk{
    Widget::~Widget(){
        
    }
    bool Widget::handle(Event& ev){
        //Default Process event
        if(ev.type() == Event::Type::SetRect){
            rect = static_cast<SetRectEvent&>(ev).rect();
            return true;
        }
        return false;
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