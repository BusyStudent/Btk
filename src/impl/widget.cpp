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
    Container::Container():EventDispatcher(widgets_list){
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
        return EventDispatcher::handle(event);
    }
}
namespace Btk{
    //EventDispatcher
    bool EventDispatcher::handle(Event &event){
        switch(event.type()){
            case Event::Click:
                return handle_click(event_cast<MouseEvent&>(event));
            case Event::KeyBoard:
                return handle_keyboard(event_cast<KeyEvent&>(event));
            case Event::Motion:
                return handle_motion(event_cast<MotionEvent&>(event));
            case Event::TextInput:
                return handle_textinput(event_cast<TextInputEvent&>(event));
            default:{
                for(auto widget:widgets){
                    if(widget->handle(event)){
                        return true;
                    }
                }
                return false;
            }
        }
    }
    bool EventDispatcher::handle_click(MouseEvent &event){
        event.accept();
        
        int x = event.x;
        int y = event.y;

        //If the focus_widget get focus by click
        if(focus_widget != nullptr){
            if(focus_widget->attr.focus == FocusPolicy::Click){
                //Is not dragging
                if(focus_widget != drag_widget and not focus_widget->rect.has_point(x,y)){
                    set_focus_widget(nullptr);
                }
            }
        }

        if(event.state == MouseEvent::Pressed){
            mouse_pressed = true;
        }
        else{
            //cleanup flags
            mouse_pressed = false;
            drag_rejected = false;
            //Dragging is at end
            if(drag_widget != nullptr){
                DragEvent ev(Event::DragEnd,x,y,-1,-1);
                drag_widget->handle(ev);
                BTK_LOGINFO("(%s)%p DragEnd",get_typename(drag_widget).c_str(),drag_widget);
                drag_widget = nullptr;
            }
        }

        if(cur_widget == nullptr){
            //try to find a new_widget
            for(auto widget:widgets){
                if(widget->visible() and widget->rect.has_point(x,y)){
                    cur_widget = widget;
                    //It can get focus by Click
                    if(widget->attr.focus == FocusPolicy::Click){
                        set_focus_widget(cur_widget);
                    }
                    break;
                }
            }
            //We didnot find it
            return true;
        }
        //Send the mouse event to it
        cur_widget->handle(event);

        if(cur_widget != focus_widget){
            if(cur_widget->attr.focus == FocusPolicy::Click){
                //It can get focus by click
                set_focus_widget(cur_widget);
            }
        }
        return true;
    }
    bool EventDispatcher::handle_motion(MotionEvent &event){
        event.accept();
        
        int x = event.x;
        int y = event.y;
        
        //Is dragging
        if(drag_widget != nullptr){
            DragEvent drag_ev(Event::Drag,event);
            BTK_LOGINFO("(%s)%p Dragging x=%d y=%d",get_typename(drag_widget).c_str(),drag_widget,x,y);
            drag_widget->handle(drag_ev);
        }
        else if(mouse_pressed == true and 
                cur_widget != nullptr and 
                not(drag_rejected)){
            

            //Send Drag Begin
            DragEvent drag_ev(Event::DragBegin,event);
            if(cur_widget->handle(drag_ev)){
                if(drag_ev.is_accepted()){
                    //The event is accepted
                    drag_widget = cur_widget;
                    BTK_LOGINFO("(%s)%p accepted DragBegin",get_typename(cur_widget).c_str(),cur_widget);
                }
                else{
                    drag_rejected = true;
                    BTK_LOGINFO("(%s)%p rejected DragBegin",get_typename(cur_widget).c_str(),cur_widget);
                }
            }
            else{
                drag_rejected = true;
                BTK_LOGINFO("(%s)%p rejected DragBegin",get_typename(cur_widget).c_str(),cur_widget);
            }
        }
        //We didnot has the widget
        if(cur_widget != nullptr){
            if(cur_widget->rect.has_point(x,y)){
                //It does not change
                //Dispatch this motion event
                cur_widget->handle(event);
                return true;
            }
            else{
                //widget leave
                event.set_type(Event::Type::Leave);
                cur_widget->handle(event);
                cur_widget = nullptr;
            }
        }
        //find new widget which has this point
        for(auto widget:widgets){
            if(widget->visible() and widget->rect.has_point(x,y)){
                cur_widget = widget;
                //widget enter
                event.set_type(Event::Type::Enter);
                cur_widget->handle(event);
                break;
            }
        }
        return true;
    }
    bool EventDispatcher::handle_keyboard(KeyEvent &event){
        //event.accept();

        if(focus_widget != nullptr){
            if(focus_widget->handle(event)){
                if(event.is_accepted()){
                    return true;
                }
            }
        }
        if(cur_widget != nullptr){
            if(cur_widget->handle(event)){
                if(event.is_accepted()){
                    return true;
                }
            }
        }
        for(auto widget:widgets){
            if(widget != focus_widget and widget != cur_widget){
                widget->handle(event);
                if(event.is_accepted()){
                    return true;
                }
            }
        }

        return false;
    }
    bool EventDispatcher::handle_textinput(TextInputEvent &event){
        event.accept();
        if(focus_widget != nullptr){
            //Send the event to focus event
            focus_widget->handle(event);
            if(not event.is_accepted()){
                //this event is rejected
                //dispatch it to other widget
                for(auto widget:widgets){
                    if(widget != focus_widget){
                        widget->handle(event);
                        if(event.is_accepted()){
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }
    void EventDispatcher::set_focus_widget(Widget *widget){
        Event event(Event::LostFocus);
        //We has last widget which has focus
        if(focus_widget != nullptr){
            //Send a lose focus
            
            BTK_LOGINFO("[EventDispatcher:%p]'%s' %p lost focus",
                        this,
                        get_typename(focus_widget).c_str(),
                        focus_widget);
            
            focus_widget->handle(event);
        }
        event.set_type(Event::TakeFocus);
        focus_widget = widget;
        
        if(widget != nullptr){

            BTK_LOGINFO("[EventDispatcher:%p]'%s' %p take focus",
                this,
                get_typename(focus_widget).c_str(),
                focus_widget);

            
            focus_widget->handle(event);
        }

    }

}