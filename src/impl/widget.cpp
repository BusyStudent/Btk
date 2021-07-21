//Widget impl
#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/container.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    Widget::Widget() = default;
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
                return handle_mouse(event_cast<MouseEvent&>(ev));
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
            case Event::DropBegin:
            case Event::DropEnd:
            case Event::DropFile:
            case Event::DropText:
                return handle_drop(event_cast<DropEvent&>(ev));
            default:
                return false;
        }
    }
    //Virtual member function
    void Widget::set_parent(Widget *parent){
        this->_parent = parent;
        //Reset the cache
        _window = nullptr;

        //Inhert style if empty
        if(_theme.empty() and _font.empty()){
            inhert_style();
        }
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
        fprintf(output,"- %s:(%d,%d,%d,%d)\n",get_typename(typeid(*this)).c_str(),x(),y(),w(),h());
        for(auto ch:childrens){
            ch->dump_tree_impl(output,depth + 1);
        }
    }
    //Hide and show
    void Widget::hide(){
        attr.hide = true;
        Event event(Event::Hide);
        handle(event);
    }
    void Widget::show(){
        attr.hide = false;
        Event event(Event::Show);
        handle(event);
    }

    void Widget::inhert_style(){
        if(parent() != nullptr){
            if( not parent()->_font.empty() and 
                not parent()->_theme.empty()){
                //Use parent
                set_font(parent()->font());
                set_theme(parent()->theme());
                return;
            }
        }
        if(window() != nullptr){
            //User window
            set_font(window()->font());
            set_theme(window()->theme());
            return;
        }
        set_theme(Themes::GetDefault());
        set_font(Font(theme().font_name(),theme().font_size()));
    }
    bool Container::add(Widget *w){
        if(w == nullptr){
            return false;
        }
        childrens.push_back(w);
        //Tell the widget
        try{
            w->set_parent(this);
        }
        catch(...){
            //Detach it
            childrens.pop_back();
            w->set_parent(nullptr);
            throw;
        }
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
namespace Btk{
    //TODO Improve The Event dispatch
    /**
     * @brief Dispatch event to widget
     * 
     * @param w The widget pointer(return false on nullptr)
     * @param event 
     * @return true 
     * @return false 
     */
    static bool dispatch_to_widget(Widget *w,Event &event){
        if(w == nullptr){
            return false;
        }
        return w->handle(event);
    }
    void Group::draw(Renderer &render){
        for(auto i = childrens.rbegin();i != childrens.rend();++i){
            Widget *w = *i;
            if(w->visible() and not w->rectangle().empty()){
                w->draw(render);
            }
        }
    }
    bool Group::handle(Event &event){
        //Broadcast to all widgets
        if(event.is_broadcast()){
            //Dispatch to children
            for(auto widget:childrens){
                widget->handle(event);
            }
            return true;
        }
        if(Widget::handle(event)){
            return true;
        }
        switch(event.type()){
            case Event::Leave:{
                //The mouse leave the widget
                bool v = dispatch_to_widget(cur_widget,event);
                cur_widget = nullptr;
                return v;
            }
            default:
                break;
        }
        return false;
    }
    bool Group::handle_drag(DragEvent &event){
        switch(event.type()){
            case Event::DragBegin:{
                BTK_LOGINFO("[%s->Group:%p]DragBegin (%d,%d)",
                    get_typename(this).c_str(),
                    this,
                    event.x,
                    event.y
                );
                
                auto widget = find_children(event.position());
                if(widget == nullptr){
                    return false;
                }
                if(widget->handle(event)){
                    if(event.accept()){
                        drag_widget = widget;
                        BTK_LOGINFO("Drag accepted");
                        return true;
                    }
                }
                BTK_LOGINFO("Drag rejected");
                return false;
            }
            case Event::Drag:{
                //Dragging
                BTK_LOGINFO("[%s->Group:%p]Drag (%d,%d)",
                    get_typename(this).c_str(),
                    this,
                    event.x,
                    event.y
                );
                dispatch_to_widget(drag_widget,event);
                return event.accept();
            }
            case Event::DragEnd:{
                //Drag is end
                BTK_LOGINFO("[%s->Group:%p]DragEnd (%d,%d)",
                    get_typename(this).c_str(),
                    this,
                    event.x,
                    event.y
                );
                dispatch_to_widget(drag_widget,event);
                drag_widget = nullptr;
                return event.accept();
            }
            default:{
                //Impossible
                BTK_ASSERT(!"Impossible");
                return false;
            }
        }
    }
    bool Group::handle_motion(MotionEvent &event){
        if(cur_widget == nullptr){
            //Try to find the widget
            cur_widget = find_children(event.position());
            if(cur_widget == nullptr){
                return false;
            }
            //Send enter
            Event levent(Event::Enter);
            cur_widget->handle(levent);
        }
        else{
            if(not cur_widget->rectangle().has_point(event.position())){
                //Leave the area
                //Send motion
                cur_widget->handle(event);
                //Send leave
                Event levent(Event::Leave);
                cur_widget->handle(levent);
                cur_widget = nullptr;

                //Try to find the widget
                cur_widget = find_children(event.position());
                if(cur_widget == nullptr){
                    return false;
                }
                //founded
                levent.set_type(Event::Enter);
                cur_widget->handle(levent);
            }
        }    
        if(cur_widget != nullptr){
            if(not cur_widget->visible()){
                //The widget is hided
                return false;
            }
        }
        //Dispatch the motion
        return cur_widget->handle(event);
    }
    bool Group::handle_mouse(MouseEvent &event){
        //In most of time,cur_widget always point at the mouse point at
        if(focus_widget == nullptr and event.is_pressed()){
            //Try to GainFocus
            if(cur_widget != nullptr){
                if(cur_widget->attribute().focus == FocusPolicy::Mouse){
                    set_focus_widget(cur_widget);
                }
            }
        }
        else if(event.is_pressed()){
            //Already has a focused widget
            if(focus_widget->attribute().focus == FocusPolicy::Mouse){
                if(not focus_widget->rectangle().has_point(event.position())){
                    //Out of the widget's range
                    //Try reset the focus
                    set_focus_widget(nullptr);
                }
            }
        }
        if(cur_widget != nullptr){
            if(not cur_widget->visible()){
                //The widget is hided
                return false;
            }
        }
        return dispatch_to_widget(cur_widget,event);
    }
    bool Group::handle_keyboard(KeyEvent &event){
        if(focus_widget == nullptr){
            return dispatch_to_widget(cur_widget,event);
        }
        return dispatch_to_widget(focus_widget,event);
    }
    bool Group::handle_wheel(WheelEvent &event){
        //TODO Add Wheel focus there
        if(focus_widget == nullptr){
            return dispatch_to_widget(cur_widget,event);
        }
        return dispatch_to_widget(focus_widget,event);
    }
    bool Group::handle_textinput(TextInputEvent &event){
        //If we donnot find any widget has focus,send to the widget mouse point at
        if(focus_widget == nullptr){
            return dispatch_to_widget(cur_widget,event);
        }
        return dispatch_to_widget(focus_widget,event);
    }
    bool Group::detach(Widget *w){
        bool val = Container::detach(w);
        if(val){
            if(w == cur_widget){
                cur_widget = nullptr;
            }
            if(w == focus_widget){
                focus_widget = nullptr;
            }
            if(w == drag_widget){
                drag_widget = nullptr;
            }
        }
        return val;
    }
    bool Group::set_focus_widget(Widget *w){
        Event event(Event::LostFocus);
        //Reset prev's focus
        dispatch_to_widget(focus_widget,event);

        #ifndef NDEBUG
        if(focus_widget != nullptr){
            BTK_LOGINFO("[%s->Group:%p]'%s' %p LostFocus",
                get_typename(this).c_str(),
                this,
                get_typename(focus_widget).c_str(),
                focus_widget
            );
        }
        #endif

        focus_widget = w;
        //Set current focus
        event.set_type(Event::TakeFocus);

        bool val = dispatch_to_widget(focus_widget,event);
        //Refuse to take focus
        if(not val or event.is_rejected()){
            focus_widget = nullptr;
            return false;
        }
        #ifndef NDEBUG
        if(focus_widget != nullptr){
            BTK_LOGINFO("[%s->Group:%p]'%s' %p TakeFocus",
                get_typename(this).c_str(),
                this,
                get_typename(focus_widget).c_str(),
                focus_widget
            );
        }
        #endif
        return true;
    }
}