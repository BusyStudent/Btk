//Widget impl
#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/core.hpp>
#include <Btk/container.hpp>
#include <Btk/widget.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace{
    struct UserDataItem{
        UserDataItem *next;
        void *data;
        char *name;

        void *operator new(std::size_t s){
            return SDL_malloc(s);
        }
        void operator delete(void *p){
            SDL_free(p);
        }
    };
    void _free_item(UserDataItem *prev,UserDataItem *cur){
        if(prev != nullptr){
            prev->next = cur->next;
        }
        SDL_free(cur->name);
        delete cur;
    }
    void _free_item_all(UserDataItem *p){
        while(p != nullptr){
            auto next = p->next;
            SDL_free(p->name);
            delete p;
            p = next;
        }
    }
}

namespace Btk{
    Widget::Widget(){
        //Set theme
        set_theme(CurrentTheme());
        //Configure widget
        set_font(theme().font);
    }
    Widget::~Widget(){
        //Delete each children
        clear_childrens();
        //Free name
        SDL_free(_name);
        //Has Items
        _free_item_all(static_cast<UserDataItem*>(_userdata));
    }
    bool Widget::handle(Event& ev){
        //Default Process event
        switch(ev.type()){
            //Normal event handle
            case Event::Resize:
                return handle_resize(event_cast<ResizeEvent&>(ev));
            case Event::Motion:
                return handle_motion(event_cast<MotionEvent&>(ev));
            case Event::Mouse:
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
            case Event::TextEditing:
                return handle_textediting(event_cast<TextEditingEvent&>(ev));
            case Event::DropBegin:
            case Event::DropEnd:
            case Event::DropFile:
            case Event::DropText:
                return handle_drop(event_cast<DropEvent&>(ev));
            //Cursor switch
            case Event::Enter:
                if(not _cursor.empty()){
                    _cursor.set();
                }
                return false;//< Let user process it
            case Event::Leave:
                if(not _cursor.empty()){
                    _cursor.reset();
                }
                return false;//< Let user process it
            default:
                return false;
        }
    }
    void Widget::set_name(u8string_view name){
        _name = static_cast<char*>(SDL_realloc(_name,name.size() + 1));
        if(_name == nullptr){
            throwSDLError();
        }
        std::memcpy(_name,name.data(),name.size());
        _name[name.size()] = '\0';
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
    void Widget::set_userdata(const char *key,void *value){
        if(is_window()){
            //We use sdl fn
            SDL_SetWindowData(static_cast<Window*>(this)->sdl_window(),key,value);
            return;
        }
        //Use our implment
        UserDataItem *cur = static_cast<UserDataItem*>(_userdata);
        UserDataItem *prev = nullptr;
        //TODO
        while(cur != nullptr){
            if(strcmp(cur->name,key) == 0){
                //Same
                if(value != nullptr){
                    cur->data = value;
                    return;
                }
                else{
                    //Remove
                    if(prev == nullptr){
                        //First item
                        _userdata = cur->next;
                    }
                    _free_item(prev,cur);
                    return;
                }
            }

            prev = cur;
            cur = cur->next;
        }
        //New a new item
        cur = new UserDataItem;
        cur->data = value;
        cur->name = SDL_strdup(key);
        cur->next = static_cast<UserDataItem*>(_userdata);
        _userdata = cur;
    }
    void *Widget::userdata(const char *name){
        if(is_window()){
            return SDL_GetWindowData(static_cast<Window*>(this)->sdl_window(),name);
        }
        UserDataItem *cur = static_cast<UserDataItem*>(_userdata);
        while(cur != nullptr){
            if(strcmp(name,cur->name) == 0){
                return cur->data;
            }
        }
        return nullptr;
    }
    void Widget::set_rect(const Rect &rect){
        this->rect = rect;    
    }
    void Widget::move(int x,int y){
        set_rect(
            x,y,
            rect.w,
            rect.h
        );
    }
    void Widget::resize(int w,int h,bool is_sizing){
        ResizeEvent event(w,h,is_sizing);
        handle(event);
        rect.w = w;
        rect.h = h;
    }
    //redraw the window
    void Widget::redraw() const{
        if(parent() == nullptr){
            //We couldnot find the window
            return;
        }
        //Try to find the window
        Window *win = window();
        if(win == nullptr){
            return;
        }
        win->Window::redraw();
    }
    void Widget::raise() const{
        Widget *w = const_cast<Widget*>(this);
        if(is_window()){
            //Self is window
            SDL_RaiseWindow(static_cast<Window*>(w)->sdl_window());
            return;
        }
        //Check has parent
        if(parent() == nullptr){
            throwRuntimeError("no parent");
        }
        if(parent()->is_container()){
            //Is dynmaic safer?
            dynamic_cast<Container*>(parent())->raise_widget(w);
        }
        else{
            throwRuntimeError("not container");
        }
    }
    //Get the window
    Renderer *Widget::renderer() const{
        if(parent() == nullptr){
            if(is_window()){
                auto w = const_cast<Widget*>(this);
                return (static_cast<Window*>(w)->_render);
            }
            return nullptr;
        }
        return (window()->_render);
    }
    //Get root
    Widget *Widget::root() const{
        Widget *cur = const_cast<Widget*>(this);
        while(cur->_parent != nullptr){
            cur = cur->_parent;
        }
        return cur;
    }
    //Try to find the window
    Window *Widget::window() const noexcept{
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
        
        if(cur->is_window()){
            //Is the window
            BTK_ASSERT_CASTABLE(Window,cur);
            _window = static_cast<Window*>(cur);
            return _window;
        }
        return nullptr;
    }
    auto Widget::find_children(u8string_view name) const -> Widget*{
        for(auto &child:childrens){
            if(name.compare(child->_name)){
                return child;
            }
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
    //Draw the bounds
    void Widget::draw_bounds(Color c){
        Renderer *r = renderer();
        if(r == nullptr){
            return;
        }
        r->save();
        r->set_antialias(false);
        r->reset_transform();

        r->begin_path();
        r->stroke_color(c);
        
        draw_bounds_impl();
        
        r->stroke();
        r->restore();
    }
    void Widget::draw_bounds_impl(){
        if(not visible()){
            return;
        }
        renderer()->rect(rectangle());
        for(auto &c:childrens){
            c->draw_bounds_impl();
        }
    }
    //Defer
    void Widget::defer_delete(){
        defer_call(&Widget::defer_delete);
    }
    void Widget::defer_delete_impl(){
        delete this;
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
    Size Widget::size_hint() const{
        return Size(-1,-1);
    }
    Size Widget::minimum_size_hint() const{
        return Size(-1,-1);
    }
    Size Widget::maximum_size_hint() const{
        return Size(-1,-1);
    }

    void Widget::inhert_style(){
        if(parent() != nullptr){
            if( not  parent()->_font.empty() and 
                not (parent()->_theme.empty())){
                //Use parent
                set_font(parent()->font());
                set_theme(parent()->_theme);
                return;
            }
        }
        if(window() != nullptr){
            //User window
            set_font(window()->font());
            set_theme(window()->_theme);
            return;
        }
        // set_theme(Themes::GetDefault());
        // set_font(Font(theme().font_name(),theme().font_size()));
    }
    bool Container::add(Widget *w){
        if(w == nullptr){
            return false;
        }
        //Probably we should check the parent
        if(w->parent() != nullptr){
            //Unallowed
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
    void Container::raise_widget(Widget *w){
        auto iter = std::find(childrens.begin(),childrens.end(),w);
        if(iter == childrens.end()){
            throwRuntimeError("unknown widget in container");
        }
        childrens.erase(iter);
        childrens.push_front(w);
        //Need i call redraw? 
    }
    void Container::lower_widget(Widget *w){
        auto iter = std::find(childrens.begin(),childrens.end(),w);
        if(iter == childrens.end()){
            throwRuntimeError("unknown widget in container");
        }
        childrens.erase(iter);
        childrens.push_back(w);
        //Need i call redraw? 
    }
    void Container::move_widget(Widget *w,long position){
        auto iter = std::find(childrens.begin(),childrens.end(),w);
        if(iter == childrens.end()){
            throwRuntimeError("unknown widget in container");
        }
        //Get widget iter by position
        auto iter_pos = _index_widget(position);
        //Insert the widget
        childrens.erase(iter);
        childrens.insert(iter_pos,w);
        //Need i call redraw?
    }
    auto Container::_index_widget(long position) const -> std::list<Widget*>::const_iterator{
        //Get widget iter by position
        auto iter_pos = childrens.begin();
        if(position < 0){
            if(position < -childrens.size()){
                throwRuntimeError("unknown widget in container");
            }

            iter_pos = --childrens.end();
            while(position != -1){
                --iter_pos;
                position ++;
            }
        }
        else{
            if(position >= childrens.size()){
                throwRuntimeError("unknown widget in container");
            }
            std::advance(iter_pos,position);
        }
        return iter_pos;
    }
    auto Container::index_widget(long position) const -> Widget*{
        auto iter = _index_widget(position);
        return *iter;
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
    void Group::draw(Renderer &render,Uint32 timestamp){
        for(auto i = childrens.rbegin();i != childrens.rend();++i){
            Widget *w = *i;
            if(w->visible() and not w->rectangle().empty()){
                w->draw(render,timestamp);
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
                    BTK_typenameof(this),
                    this,
                    event.x,
                    event.y
                );
                
                auto widget = find_visible_children(event.position());
                if(widget == nullptr){
                    //No finded
                    BTK_LOGINFO("[%s->Group:%p]Nothing finded, leaveing...",
                        BTK_typenameof(this),
                        this
                    );
                    return false;
                }
                if(widget->handle(event)){
                    if(event.accept()){
                        drag_widget = widget;
                        BTK_LOGINFO("Drag accepted");
                        return true;
                    }
                }
                BTK_LOGINFO("[%s->Group:%p]DragRejected %s => %p",
                    BTK_typenameof(this),
                    this,
                    BTK_typenameof(widget),
                    widget
                );
                return false;
            }
            case Event::Drag:{
                //Dragging
                BTK_LOGINFO("[%s->Group:%p]Drag (%d,%d)",
                    BTK_typenameof(this),
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
                    BTK_typenameof(this),
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
    bool Group::handle_drop(DropEvent &event){
        Widget *w = find_visible_children(event.position());
        if(w != nullptr){
            BTK_LOGINFO("[Drop]Send Drop to '%s' %p",BTK_typenameof(w),w);
            return w->handle(event);
        }
        return false;
    }
    bool Group::handle_motion(MotionEvent &event){
        if(cur_widget == nullptr){
            //Try to find the widget
            cur_widget = find_visible_children(event.position());
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
                cur_widget = find_visible_children(event.position());
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
                if(cur_widget->attribute().focus_policy == FocusPolicy::Mouse){
                    set_focus_widget(cur_widget);
                }
            }
        }
        else if(event.is_pressed()){
            //Already has a focused widget
            if(focus_widget->attribute().focus_policy == FocusPolicy::Mouse){
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
    bool Group::handle_textediting(TextEditingEvent &event){
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
        return val;
    }
    //TODO: Still need improve
    void Group::invalidate(){
        if(window() == nullptr){
            return;
        }
        //Check current mouse point widget
        auto pos = window()->mouse_position();
        if(cur_widget != nullptr){
            //Make a leave event
            Event levent(Event::Leave);
            cur_widget->handle(levent);
            cur_widget = nullptr;
        }
        if(focus_widget != nullptr){
            set_focus_widget(nullptr);
        }
        focus_widget = nullptr;
        drag_widget = nullptr;
    }
}