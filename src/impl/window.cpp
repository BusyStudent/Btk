#include <SDL2/SDL_image.h>

#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
#include <Btk/widget.hpp>
#include <Btk/layout.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    static SDL_Window *CreateWindow(const char *title,int x,int y,int w,int h,int flags){
        SDL_Window *win = SDL_CreateWindow(title,x,y,w,h,flags);
        if(win == nullptr){
            const char *err = SDL_GetError();
            BTK_LOGINFO("%s",err);
            throwSDLError(err);
        }
        return win;
    }

    WindowImpl::WindowImpl(const char *title,int x,int y,int w,int h,int flags):
         win(CreateWindow(title,x,y,w,h,flags)),
         render(win){
        //Set theme
        theme = Themes::GetDefault();
        //Set background color
        bg_color = theme[Theme::Window];        
        attr.window = true;
    }
    WindowImpl::~WindowImpl(){
        //Delete widgets
        clear_childrens();
        SDL_FreeCursor(cursor);
        //destroy the render before destroy the window
        render.destroy();

        SDL_DestroyWindow(win);
    }
    //Draw window
    void WindowImpl::draw(Renderer &render){
        #ifndef NDEBUG
        SDL_Log("[System::Renderer]Draw Window %p",win);
        #endif
        std::lock_guard<std::recursive_mutex> locker(mtx);
        render.begin();
        render.clear(bg_color);
        //Draw each widget
        for(auto widget:childrens){
            //check widgets
            if((widget->visible()) and not(widget->rect.empty())){
                widget->draw(render);
            }
        }
        //Run the draw callback
        auto iter = draw_cbs.begin();
        while(iter != draw_cbs.end()){
            if(not iter->call(render)){
                //Remove it
                iter = draw_cbs.erase(iter);
            }
        }
        render.end();
    }
    void WindowImpl::redraw(){
        if(not visible){
            //Window is not visible
            return;
        }
        auto current = SDL_GetTicks();
        if(fps_limit > 0){
            //Has limit
            Uint32 durl = 1000 / fps_limit;
            if(last_redraw_ticks - current < durl){
                //Too fast,ignore it
                BTK_LOGINFO("Drawing too fast,ignored");
                return;
            }
        }
        //Update it
        last_redraw_ticks = current;
        
        SDL_Event event;
        event.type = SDL_WINDOWEVENT;
        event.window.timestamp = current;
        event.window.windowID = SDL_GetWindowID(win);
        event.window.event = SDL_WINDOWEVENT_EXPOSED;
        SDL_PushEvent(&event);
       
    }
    //TryCloseWIndow
    bool WindowImpl::on_close(){
        if(not sig_close.empty()){
            return sig_close();
        }
        return true;
    }
    //DropFilecb
    void WindowImpl::on_dropfile(u8string_view file){
        if(not sig_dropfile.empty()){
            sig_dropfile(file);
        }
    }
    void WindowImpl::on_resize(int new_w,int new_h){
        if(not sig_resize.empty()){
            sig_resize(new_w,new_h);
        }
    }
    void WindowImpl::pixels_size(int *w,int *h){
        SDL_GetWindowSize(win,w,h);
        
        //SDL_GL_GetDrawableSize(win,w,h);
    }
    void WindowImpl::buffer_size(int *w,int *h){
        auto size = render.output_size();
        if(w != nullptr){
            *w = size.w;
        }
        if(h != nullptr){
            *h = size.h;
        }
    }
    //update widgets postions
    void WindowImpl::update_postion(){
        auto &widgets_list = childrens;
        if(widgets_list.empty()){
            return;
        }
        if(widgets_list.size() == 1){
            //only have on widget
            //set it x = 0 y = 0 
            //w = window's w
            //h = window's h
            Widget *widget = *widgets_list.begin();
            if(not widget->attr.user_rect){
                int w,h;
                pixels_size(&w,&h);
                widget->set_rect(0,0,w,h);
            }
        }
        else{
            //not to change widgets postion
            //update each Layout
            Layout *layout;
            for(auto widget:widgets_list){
                layout = dynamic_cast<Layout*>(widget);
                if(layout != nullptr){
                    layout->update();
                }
            }
        }
    }
    //Dispatch Event
    bool WindowImpl::dispatch(Event &event){
        if(Widget::handle(event)){
            if(event.is_accepted()){
                return true;
            }
            return false;
        }
        else{
            //Using the EventSignal
            if(sig_event.empty()){
                return false;
            }
            return sig_event(event);
        }
    }
}
//Event Processing
namespace Btk{
    bool WindowImpl::handle(Event &event){
        if(not Widget::handle(event)){
            //The widget doesnnot process it
            return sig_event(event);
        }
        return true;
    }
    void WindowImpl::handle_windowev(const SDL_Event &event){
        switch(event.window.event){
            case SDL_WINDOWEVENT_EXPOSED:{
                //redraw window
                draw(render);
                break;
            }
            case SDL_WINDOWEVENT_HIDDEN:{
                visible = false;
                break;
            }
            case SDL_WINDOWEVENT_SHOWN:{
                visible = true;
                break;
            }
            case SDL_WINDOWEVENT_CLOSE:{
                //Close window;
                if(on_close()){
                    //success to close
                    //Delete Wnidow;
                    System::instance->unregister_window(this);
                    if(System::instance->wins_map.empty()){
                        //No window exist
                        Btk::Exit(0);
                    }
                }
                break;
            }
            case SDL_WINDOWEVENT_RESIZED:{
                //window resize
                on_resize(event.window.data1,event.window.data2);
                break;
            }
            case SDL_WINDOWEVENT_ENTER:{
                std::lock_guard<std::recursive_mutex> locker(mtx);
                if(cursor != nullptr){
                    //has windows cursor
                    //set cursor
                    SDL_SetCursor(cursor);
                }
                Event enter(Event::WindowEnter);
                handle(enter);
                if(not sig_enter.empty()){
                    sig_enter();
                }
                break;
            }
            case SDL_WINDOWEVENT_LEAVE:{
                std::lock_guard<std::recursive_mutex> locker(mtx);
                if(cursor != nullptr){
                    //reset to system cursor
                    SDL_SetCursor(SDL_GetDefaultCursor());
                }
                //Event leave(Event::WindowLeave);
                //container.handle(leave);
                window_mouse_leave();
                if(not sig_leave.empty()){
                    sig_leave();
                }
                break;
            }
        }
    }
    bool WindowImpl::handle_click(MouseEvent &event){
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

                SDL_CaptureMouse(SDL_FALSE);
            }
        }

        if(cur_widget == nullptr){
            //try to find a new_widget
            for(auto widget:childrens){
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
    bool WindowImpl::handle_motion(MotionEvent &event){
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
        for(auto widget:childrens){
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
    bool WindowImpl::handle_keyboard(KeyEvent &event){
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
        for(auto widget:childrens){
            if(widget != focus_widget and widget != cur_widget){
                widget->handle(event);
                if(event.is_accepted()){
                    return true;
                }
            }
        }

        return false;
    }
    bool WindowImpl::handle_textinput(TextInputEvent &event){
        event.accept();
        if(focus_widget != nullptr){
            //Send the event to focus event
            focus_widget->handle(event);
            if(not event.is_accepted()){
                //this event is rejected
                //dispatch it to other widget
                for(auto widget:childrens){
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
    bool WindowImpl::handle_wheel(WheelEvent &event){
        if(cur_widget != nullptr){
            return cur_widget->handle(event);
        }
        return false;
    }
    void WindowImpl::set_focus_widget(Widget *widget){
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
    void WindowImpl::window_mouse_leave(){
        if(cur_widget != nullptr){
            Event event(Event::Leave);
            cur_widget->handle(event);
            cur_widget = nullptr;
        }
    }

    
}
namespace Btk{
    Window::Window(u8string_view title,int w,int h,Flags f){
        Init();
        #ifdef __ANDROID__
        //Android need full screen
        Uint32 flags = 
            SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL;
        #else
        Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;
        #endif

        if((f & Flags::OpenGL) == Flags::OpenGL){
            flags |= SDL_WINDOW_OPENGL;
        }
        if((f & Flags::Vulkan) == Flags::Vulkan){
            flags |= SDL_WINDOW_VULKAN;
        }
        if((f & Flags::SkipTaskBar) == Flags::SkipTaskBar){
            flags |= SDL_WINDOW_SKIP_TASKBAR;
        }
        

        pimpl = new WindowImpl(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,
            h,
            flags
        );
        // pimpl->container.window = pimpl;
        SDL_SetWindowData(pimpl->win,"btk_win",this);
        SDL_SetWindowData(pimpl->win,"btk_imp",pimpl);
        winid = SDL_GetWindowID(pimpl->win);
        System::instance->register_window(pimpl);
    }
    bool Window::mainloop(){
        done();
        return Btk::run() == 0;
    }
    Window::SignalClose &Window::signal_close(){
        return pimpl->sig_close;
    }
    Window::SignalEvent &Window::signal_event(){
        return pimpl->sig_event;
    }
    Window::SignalResize &Window::signal_resize(){
        return pimpl->sig_resize;
    }
    Window::SignalDropFile& Window::signal_dropfile(){
        return pimpl->sig_dropfile;
    }
    void Window::set_title(u8string_view title){
        SDL_SetWindowTitle(pimpl->win,title.data());
    }
    void Window::draw(){
        pimpl->redraw();
    }
    void Window::close(){
        //send a close request
        SDL_Event event;
        SDL_zero(event);
        event.type = SDL_WINDOWEVENT;
        event.window.timestamp = SDL_GetTicks();
        event.window.windowID = winid;
        event.window.event = SDL_WINDOWEVENT_CLOSE;
        SDL_PushEvent(&event);
    }
    void Window::move(int x,int y){
        SDL_SetWindowPosition(pimpl->win,x,y);
    }
    
    void Window::set_icon(u8string_view file){
        SDL_Surface *image = IMG_Load(file.data());
        if(image == nullptr){
            throwSDLError(IMG_GetError());
        }
        SDL_SetWindowIcon(pimpl->win,image);
        SDL_FreeSurface(image);
    }
    void Window::set_icon(const PixBuf &pixbuf){
        SDL_SetWindowIcon(pimpl->win,pixbuf.get());
    }

    PixBuf Window::pixbuf(){
        return SDL_GetWindowSurface(pimpl->win);
    }
    
    int Window::w() const noexcept{
        int w;
        SDL_GetWindowSize(pimpl->win,&w,nullptr);
        return w;
    }
    int Window::h() const noexcept{
        int h;
        SDL_GetWindowSize(pimpl->win,nullptr,&h);
        return h;
    }
    //add widget
    bool Window::add(Widget *ptr){
        return pimpl->add(ptr);
    }
    //Window exists
    bool Window::exists() const{
        //Lock maps
        return Instance().get_window_s(winid) == pimpl;
    }
    //update widgets postions
    void Window::update(){
        pimpl->update_postion();
    }
    //Set cursor
    void Window::set_cursor(){
        SDL_FreeCursor(pimpl->cursor);
        pimpl->cursor = SDL_CreateSystemCursor(
            SDL_SYSTEM_CURSOR_ARROW
        );
        SDL_Window *win = SDL_GetMouseFocus();
        if(win != nullptr){
            if(winid == SDL_GetWindowID(win)){
                //Window has focus
                //reset cursor right now
                SDL_SetCursor(pimpl->cursor);
            }
        }
    }
    void Window::set_cursor(const PixBuf &pixbuf,int hotx,int hoty){
        SDL_FreeCursor(pimpl->cursor);
        pimpl->cursor = SDL_CreateColorCursor(
            pixbuf.get(),hotx,hoty
        );
        SDL_Window *win = SDL_GetMouseFocus();
        if(win != nullptr){
            if(winid == SDL_GetWindowID(win)){
                //Window has focus
                //reset cursor right now
                SDL_SetCursor(pimpl->cursor);
            }
        }
    }
    void Window::set_fullscreen(bool val){
        Uint32 flags;
        if(val){
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else{
            flags = 0;
        }
        if(SDL_SetWindowFullscreen(pimpl->win,flags) != 0){
            throwSDLError();
        }
    }
    //set resize able
    void Window::set_resizeable(bool val){
        SDL_bool s_val;
        if(val){
            s_val = SDL_TRUE;
        }
        else{
            s_val = SDL_FALSE;
        }
        SDL_SetWindowResizable(pimpl->win,s_val);
    }
    //multi threading
    void Window::lock(){
        pimpl->mtx.lock();
    }
    void Window::unlock(){
        pimpl->mtx.unlock();
    }
    //Show window and set Widget postions
    void Window::done(){
        update();
        SDL_ShowWindow(pimpl->win);
    }
    Font Window::font() const{
        throwRuntimeError("Unimpl yet");
    }
    void Window::dump_tree(FILE *output) const{
        pimpl->dump_tree(output);
    }
}
namespace Btk{
    Size GetScreenSize(int index){
        SDL_DisplayMode mode;
        if(SDL_GetCurrentDisplayMode(index,&mode) == -1){
            throwSDLError();
        }
        return {mode.w,mode.h};
    }
}