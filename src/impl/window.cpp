#include <SDL2/SDL_image.h>

#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
#include <Btk/widget.hpp>
#include <Btk/layout.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    
    WindowImpl::WindowImpl(const char *title,int x,int y,int w,int h,int flags){
        refcount = 0;//default refcount
        //Btk::Window doesnnot has it ownship
        win = nullptr;
        render = nullptr;
        win = SDL_CreateWindow(title,x,y,w,h,flags);
        if(win == nullptr){
            //Handle err....
            throwSDLError();
        }
        render = SDL_CreateRenderer(win,-1,SDL_RENDERER_ACCELERATED);
        if(render == nullptr){
            //try software render
            render = SDL_CreateRenderer(win,-1,0);
            if(render == nullptr){
                throwSDLError();
            }
            
        }
        cursor = nullptr;
        //Set theme
        theme = &Themes::GetDefault();
        //Open DefaultFont
        default_font.open(theme->font,theme->font_ptsize);
        //Set background color
        bg_color = theme->background_color;

        last_draw_ticks = 0;
        //Init it to -1
        mouse_x = -1;
        mouse_y = -1;
        cur_widget = nullptr;
    }
    WindowImpl::~WindowImpl(){
        //Delete widgets
        for(auto widget:widgets_list){
            delete widget;
        }
        SDL_FreeCursor(cursor);
        SDL_DestroyRenderer(*render);
        SDL_DestroyWindow(win);
        render = nullptr;
    }
    //Draw window
    void WindowImpl::draw(){
        auto current = SDL_GetTicks();
        if(current < last_draw_ticks + 10){
            //drawing too fast
            last_draw_ticks = current;
            return;
        }
        else{
            last_draw_ticks = current;
        }
        #ifndef NDEBUG
        SDL_Log("[System::Renderer]Draw Window %p",win);
        #endif
        std::lock_guard<std::recursive_mutex> locker(mtx);
        render.start(bg_color);
        //Draw each widget
        for(auto widget:widgets_list){
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
        render.done();
    }
    //TryCloseWIndow
    bool WindowImpl::on_close(){
        if(not sig_close.empty()){
            return sig_close();
        }
        return true;
    }
    //DropFilecb
    void WindowImpl::on_dropfile(std::string_view file){
        if(not sig_dropfile.empty()){
            sig_dropfile(file);
        }
    }
    void WindowImpl::on_resize(int new_w,int new_h){
        if(not sig_resize.empty()){
            sig_resize(new_w,new_h);
        }
    }
    void WindowImpl::ref(){
        refcount ++;
    }
    void WindowImpl::unref(){
        refcount--;
        if(refcount == 0){
            delete this;
        }
    }
    void WindowImpl::pixels_size(int *w,int *h){
        //SDL_GetWindowSize(win,w,h);
        SDL_GetRendererOutputSize(*render,w,h);
    }
    //update widgets postions
    void WindowImpl::update_postion(){
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
                SetRectEvent ev({
                    0,
                    0,
                    w,
                    h
                });
                widget->handle(ev); 
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
        bool flags = false;
        for(auto widget:widgets_list){
            flags |= widget->handle(event);
            if(event.is_accepted()){
                return flags;
            }
        }
        //No any widget accept it
        if(not sig_event.empty()){
            flags = sig_event(event);
        }
        return flags;
    }
}
//Event Processing
namespace Btk{
    void WindowImpl::handle_windowev(const SDL_Event &event){
        switch(event.window.event){
            case SDL_WINDOWEVENT_EXPOSED:{
                //redraw window
                draw();
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
                if(not sig_leave.empty()){
                    sig_leave();
                }
                break;
            }
        }
    }
    //mouse motion
    void WindowImpl::handle_mousemotion(const SDL_Event &event){
        //Get new x and y
        int x = event.motion.x;
        int y = event.motion.y;

        MotionEvent ev = TranslateEvent(event.motion);

        //Is dragging
        if(drag_widget != nullptr){
            DragEvent drag_ev(Event::Drag,ev);
            drag_widget->handle(drag_ev);
        }
        else if(mouse_pressed == true and 
                cur_widget != nullptr and 
                not(drag_rejected)){
            //Send Drag Begin
            DragEvent drag_ev(Event::DragBegin,ev);
            if(cur_widget->handle(drag_ev)){
                if(drag_ev.is_accepted()){
                    //The event is accepted
                    drag_widget = cur_widget;
                    BTK_LOGINFO("%p accepted DragBegin",cur_widget);
                }
                else{
                    drag_rejected = true;
                    BTK_LOGINFO("%p rejected DragBegin",cur_widget);
                }
            }
            else{
                drag_rejected = true;
                BTK_LOGINFO("%p rejected DragBegin",cur_widget);
            }
        }

        if(cur_widget != nullptr){
            if(cur_widget->rect.has_point(x,y)){
                //It does not change
                //Dispatch this motion event
                cur_widget->handle(ev);
                return;
            }
            else{
                //widget leave
                ev.set_type(Event::Type::Leave);
                cur_widget->handle(ev);
                cur_widget = nullptr;
            }
        }
        //find new widget which has this point
        for(auto widget:widgets_list){
            if(widget->rect.has_point(x,y)){
                cur_widget = widget;
                //widget enter
                ev.set_type(Event::Type::Enter);
                cur_widget->handle(ev);
                break;
            }
        }
    }
    //MouseButton down or up
    void WindowImpl::handle_mousebutton(const SDL_Event &event){
        int x = event.button.x;
        int y = event.button.y;
        //Focus handling
        if(focus_widget != nullptr){
            if(not focus_widget->rect.has_point(x,y)){
                //The widget lost the focus
                Event ev(Event::LostFocus);
                if(focus_widget->handle(ev)){
                    if(ev.is_accepted()){
                        BTK_LOGINFO("Widget %p lost focus",focus_widget);
                        focus_widget = nullptr;
                    }
                    else{
                        BTK_LOGINFO("Widget %p refused lost focus",focus_widget);
                    }
                }
                else{
                    //This widget refused to lost focus
                    BTK_LOGINFO("Widget %p refused lost focus",focus_widget);
                }

            }
            BTK_LOGINFO("Widget %p has focus",focus_widget);
        }

        if(event.button.state == SDL_PRESSED){
            mouse_pressed = true;
        }
        else{
            //clean flags
            mouse_pressed = false;
            drag_rejected = false;
            //The drag is end
            if(drag_widget != nullptr){
                DragEvent ev(Event::DragEnd,x,y,-1,-1);
                drag_widget->handle(ev);
                
                drag_widget = nullptr;
            }
        }
        if(cur_widget == nullptr){
            for(auto widget:widgets_list){
                if(widget->rect.has_point(x,y)){
                    //We find it
                    cur_widget = widget;
                    goto find;
                }
            }
            return;
        }
        find:
        auto ev = TranslateEvent(event.button);
        //Focus handling
        cur_widget->handle(ev);
        //reset the flags
        ev.reject();
        ev.set_type(Event::TakeFocus);

        if(focus_widget == nullptr){
            if(cur_widget->handle(ev)){
                if(ev.is_accepted()){
                    //The widget take the focus
                    focus_widget = cur_widget;
                    BTK_LOGINFO("Focus Widget = %p",focus_widget);
                }
            }
        }


    }
    //keyboard event
    void WindowImpl::handle_keyboardev(KeyEvent &event){
        
        if(focus_widget != nullptr){
            if(focus_widget->handle(event)){
                if(event.is_accepted()){
                    return;
                }
            }
        }
        if(cur_widget != nullptr){
            if(cur_widget->handle(event)){
                if(event.is_accepted()){
                    return;
                }
            }
        }
        for(auto widget:widgets_list){
            if(widget != focus_widget and widget != cur_widget){
                widget->handle(event);
                if(event.is_accepted()){
                    return;
                }
            }
        }
        if(not sig_event.empty()){
            sig_event(event);
        }
    }
    void WindowImpl::handle_textinput(TextInputEvent &event){
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
                            return;
                        }
                    }
                }
            }
        }
        else{
            dispatch(event);
        }
    }
}
namespace Btk{
    Window::Window(std::string_view title,int w,int h){
        Init();
        #ifdef __ANDROID__
        //Android need full screen
        constexpr Uint32 flags = 
            SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP;
        #else
        constexpr Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN;
        #endif
        pimpl = new WindowImpl(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,
            h,
            flags
        );
        SDL_SetWindowData(pimpl->win,"btk_win",this);
        SDL_SetWindowData(pimpl->win,"btk_imp",pimpl);
        winid = SDL_GetWindowID(pimpl->win);
        System::instance->register_window(pimpl);
    }
    bool Window::mainloop(){
        done();
        return Btk::run() == 0;
    }
    Window::SignalClose &Window::sig_close(){
        return pimpl->sig_close;
    }
    Window::SignalEvent &Window::sig_event(){
        return pimpl->sig_event;
    }
    Window::SignalResize &Window::sig_resize(){
        return pimpl->sig_resize;
    }
    Window::SignalDropFile& Window::sig_dropfile(){
        return pimpl->sig_dropfile;
    }
    void Window::set_title(std::string_view title){
        SDL_SetWindowTitle(pimpl->win,title.data());
    }
    void Window::draw(){
        /*
        //send a draw request
        //README possible memory error on here if window is closed
        System::instance->defer_call([](void *win){
            static_cast<WindowImpl*>(win)->draw();
        },pimpl);
        */
       if(pimpl->visible){
            //Window is not visible
            SDL_Event event;
            SDL_zero(event);
            event.type = SDL_WINDOWEVENT;
            event.window.timestamp = SDL_GetTicks();
            event.window.windowID = winid;
            event.window.event = SDL_WINDOWEVENT_EXPOSED;
            SDL_PushEvent(&event);
       }
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
    
    void Window::set_icon(std::string_view file){
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
        if(ptr == nullptr){
            return false;
        }
        else{
            pimpl->widgets_list.push_back(ptr);
            //set master
            ptr->win = this;
            return true;
        }
    }
    //Window exists
    bool Window::exists() const{
        //Lock maps
        std::lock_guard<std::recursive_mutex> locker(
            System::instance->map_mtx
        );
        auto iter = System::instance->wins_map.find(winid);
        if(iter == System::instance->wins_map.end()){
            return false;
        }
        else if(iter->second == pimpl){
            return true;
        }
        return false;
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
        return pimpl->default_font;
    }
}