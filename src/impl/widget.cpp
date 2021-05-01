//Widget impl
#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    Widget::Widget() = default;
    Widget::Widget(Widget *parent):Widget(){
        set_parent(parent);
    }
    Widget::~Widget(){
        //Delete each children
        clear_childrens();
    }
    bool Widget::handle(Event& ev){
        //Default Process event
        switch(ev.type()){
            //Normal event handle
            case Event::Motion:
                return handle_motion(event_cast<MotionEvent&>(ev));
            case Event::Click:
                return handle_click(event_cast<ClickEvent&>(ev));
            case Event::KeyBoard:
                return handle_keyboard(event_cast<KeyEvent&>(ev));
            case Event::Wheel:
                return handle_wheel(event_cast<WheelEvent&>(ev));
            case Event::Drag:
            case Event::DragBegin:
            case Event::DragEnd:
                return handle_drag(event_cast<DragEvent&>(ev));
            case Event::TextInput:
                return handle_textinput(event_cast<TextInputEvent&>(ev));
            default:
                return false;
        }
    }
    //Virtual member function
    void Widget::set_parent(Widget *parent){
        this->_parent = parent;
        //Reset the cache
        _window = nullptr;
    }
    void Widget::set_rect(const Rect &rect){
        this->rect = rect;    
    }
    //redraw the window
    void Widget::redraw(){
        if(parent() == nullptr){
            //We couldnot find the window
            return;
        }
        //Try to find the window
        WindowImpl *win = window();
        if(win == nullptr){
            return;
        }
        win->WindowImpl::redraw();
    }
    //Get the window
    Window &Widget::master() const{
        auto *pimpl = window();
        Window *win = static_cast<Window*>(SDL_GetWindowData(pimpl->win,"btk_win"));
        //This should not happended
        BTK_ASSERT(win == nullptr);
        return *win;
    }
    Renderer *Widget::renderer() const{
        if(parent() == nullptr){
            return nullptr;
        }
        return &(window()->render);
    }
    //Get the default font
    //TODO create a global default font
    Font Widget::default_font() const{
        BTK_ASSERT(parent() != nullptr);
        //return window()->font();
        throwRuntimeError("Unimpl yet");
    }
    Theme &Widget::window_theme() const{
        if(window() == nullptr){
            return Themes::GetDefault();
        }
        return window()->theme;
    }
    //Try to find the window
    WindowImpl *Widget::window() const noexcept{
        if(_window != nullptr){
            //Try the cache
            return _window;
        }
        if(parent() == nullptr){
            //No window
            return nullptr;
        }
        Widget *cur = const_cast<Widget*>(this);
        while(cur->_parent != nullptr){
            cur = cur->_parent;
        }
        //At the top
        
        if(cur->attr.window){
            //Is the window
            _window = static_cast<WindowImpl*>(cur);
            BTK_ASSERT(dynamic_cast<WindowImpl*>(cur) != nullptr);
            return _window;
        }
        return nullptr;
    }
    void Widget::clear_childrens(){
        for(auto i = childrens.begin();i != childrens.end();){
            delete *i;
            i = childrens.erase(i);
        }
    }
    void Widget::dump_tree(FILE *output){
        if(output == nullptr){
            output = stderr;
        }
        dump_tree_impl(output,0);
    }
    void Widget::dump_tree_impl(FILE *output,int depth){
        //Print space
        for(int i = 0;i < depth;i++){
            fputc(' ',output);
            fputc(' ',output);
        }
        fprintf(output,"- %s\n",get_typename(typeid(*this)).c_str());
        for(auto ch:childrens){
            ch->dump_tree_impl(output,depth + 1);
        }
    }
    bool Container::add(Widget *w){
        if(w == nullptr){
            return false;
        }
        childrens.push_back(w);
        //Tell the widget
        w->set_parent(this);
        return true;
    }
    void Container::clear(){
        clear_childrens();
    }
    //detach widget
    bool Container::detach(Widget *widget){
        if(widget == nullptr){
            return false;
        }
        auto iter = std::find(childrens.begin(),childrens.end(),widget);
        if(iter == childrens.end()){
            return false;
        }
        childrens.erase(iter);
        //Tell the widget
        widget->set_parent(nullptr);

        return true;
    }
    bool Container::remove(Widget *widget){
        if(detach(widget)){
            delete widget;
            return true;
        }
        return false;
    }
}
#if 0
namespace Btk{
    //EventDispatcher in Container
    bool Container::handle(Event &event){
        //Filt the event
        if(not ev_filter.empty()){
            if(ev_filter(event) == false){
                BTK_LOGINFO("Drop %s:%p",get_typename(&event).c_str(),&event);
                event.reject();
                return false;
            }
        }

        switch(event.type()){
            case Event::Click:
                return handle_click(event_cast<MouseEvent&>(event));
            case Event::KeyBoard:
                return handle_keyboard(event_cast<KeyEvent&>(event));
            case Event::Motion:
                return handle_motion(event_cast<MotionEvent&>(event));
            case Event::TextInput:
                return handle_textinput(event_cast<TextInputEvent&>(event));
            /*This three event handler is used to forward dragevent*/
            case Event::DragBegin:{
                auto &drag = event_cast<DragEvent&>(event);
                //try to find a widget which has the point
                for(auto widget:widgets_list){
                    if(widget->rect.has_point(drag.x,drag.y)){
                        bool ret = false;
                        ret |= widget->handle(drag);
                        ret |= drag.is_accepted();
                        if(ret){
                            drag_widget = widget;
                            return true;
                        }
                    }
                }
                return false;
            }
            case Event::DragEnd:{
                //finish dragging
                BTK_ASSERT(drag_widget != nullptr);
                bool ret = drag_widget->handle(event);
                drag_widget = nullptr;
                return ret;
            }
            case Event::Drag:{
                //Send it to drag widget
                BTK_ASSERT(drag_widget != nullptr);
                return drag_widget->handle(event);
            }
            case Event::Wheel:{
                return handle_whell(event_cast<WheelEvent&>(event));
            }
            case Event::WindowLeave:{
                #if 0
                if(cur_widget != drag_widget and cur_widget != nullptr and managed_window){
                    //On the window
                    int w,h;
                    window->pixels_size(&w,&h);
                    if(cur_widget->rect.w >= w and cur_widget->rect.h >= h){
                        //We should generate a leave event fot it
                        Event leave(Event::Leave);
                        cur_widget->handle(leave);
                        cur_widget = nullptr;
                    }
                }
                #endif
                //Boardcast to all the widget
                #if 0
                for(auto widget:widgets_list){
                    widget->handle(event);
                }
                #endif
                return true;
            }
            case Event::SetRect:{
                auto &ev = event_cast<SetRectEvent&>(event);
                return handle_setrect(ev);
            }
            case Event::SetContainer:{
                auto &ev = event_cast<SetContainerEvent&>(event);
                parent = ev.container();
            }
            default:{
                for(auto widget:widgets_list){
                    if(widget->handle(event)){
                        return true;
                    }
                }
                return false;
            }
        }
    }
    bool Container::handle_click(MouseEvent &event){
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
            if(drag_widget != nullptr and managed_window){
                DragEvent ev(Event::DragEnd,x,y,-1,-1);
                drag_widget->handle(ev);
                BTK_LOGINFO("(%s)%p DragEnd",get_typename(drag_widget).c_str(),drag_widget);
                drag_widget = nullptr;

                SDL_CaptureMouse(SDL_FALSE);
            }
        }

        if(cur_widget == nullptr){
            //try to find a new_widget
            for(auto widget:widgets_list){
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
    bool Container::handle_motion(MotionEvent &event){
        event.accept();
        
        int x = event.x;
        int y = event.y;

        //only top window can gen DragEvent
        if(managed_window){

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
                        SDL_CaptureMouse(SDL_TRUE);
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
        for(auto widget:widgets_list){
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
    bool Container::handle_keyboard(KeyEvent &event){
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
        for(auto widget:widgets_list){
            if(widget != focus_widget and widget != cur_widget){
                widget->handle(event);
                if(event.is_accepted()){
                    return true;
                }
            }
        }

        return false;
    }
    bool Container::handle_textinput(TextInputEvent &event){
        event.accept();
        if(focus_widget != nullptr){
            //Send the event to focus event
            focus_widget->handle(event);
            if(not event.is_accepted()){
                //this event is rejected
                //dispatch it to other widget
                for(auto widget:widgets_list){
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
    bool Container::handle_whell(WheelEvent &event){
        if(cur_widget != nullptr){
            return cur_widget->handle(event);
        }
        return false;
    }
    void Container::set_focus_widget(Widget *widget){
        Event event(Event::LostFocus);
        //We has last widget which has focus
        if(focus_widget != nullptr){
            //Send a lose focus
            
            BTK_LOGINFO("[Container:%p]'%s' %p lost focus",
                        this,
                        get_typename(focus_widget).c_str(),
                        focus_widget);
            
            focus_widget->handle(event);
        }
        event.set_type(Event::TakeFocus);
        focus_widget = widget;
        
        if(widget != nullptr){

            BTK_LOGINFO("[Container:%p]'%s' %p take focus",
                this,
                get_typename(focus_widget).c_str(),
                focus_widget);

            
            focus_widget->handle(event);
        }

    }
    void Container::window_mouse_leave(){
        if(cur_widget != nullptr){
            Event event(Event::Leave);
            cur_widget->handle(event);
            cur_widget = nullptr;
        }
    }
}
#endif
