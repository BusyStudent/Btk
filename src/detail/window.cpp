#include "../build.hpp"

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_timer.h>

#include <Btk/platform/platform.hpp>
#include <Btk/detail/window.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/detail/core.hpp>
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
    Window::Window(u8string_view title,int w,int h,Flags f){
        Init();

        auto sdl_win = CreateWindow(title,w,h,f);
        initlialize(sdl_win);

        win_flags = f;
    }
    Window::Window(const NativeWindow *native_handle){
        SDL_Window *sdl_win = SDL_CreateWindowFrom(native_handle);
        if(sdl_win == nullptr){
            throwSDLError();
        }
        initlialize(sdl_win);
    }
    bool Window::mainloop(){
        done();
        return Btk::run() == 0;
    }
    void Window::set_title(u8string_view title){
        SDL_SetWindowTitle(win,title.data());
    }
    void Window::resize(int w,int h){
        SDL_SetWindowSize(win,w,h);
    }
    
    void Window::set_icon(u8string_view file){
        return set_icon(PixBuf::FromFile(file));
    }
    void Window::set_icon(const PixBuf &pixbuf){
        SDL_SetWindowIcon(win,pixbuf.get());
    }

    PixBuf Window::pixbuf(){
        return SDL_GetWindowSurface(win);
    }
    
    int Window::w() const noexcept{
        int w;
        SDL_GetWindowSize(win,&w,nullptr);
        return w;
    }
    int Window::h() const noexcept{
        int h;
        SDL_GetWindowSize(win,nullptr,&h);
        return h;
    }
    void Window::initlialize(SDL_Window *_win){
        winid = SDL_GetWindowID(_win);
        win = _win;

        attr.window = true;

        //Construct renderer
        _device = CreateDevice(_win);
        if(_device == nullptr){
            throwRendererError("Couldnot create device");
        }
        _render = new Renderer(*_device);

        //Set background color
        bg_color = theme().active.window;        
        attr.window = true;

        //Update window rect
        rect.x = 0;
        rect.y = 0;
        SDL_GetWindowSize(win,&(rect.w),&(rect.h));
        //Set window data
        SDL_SetWindowData(win,"btk_impl",this);
        SDL_SetWindowData(win,"btk_dev",_device);

        #ifndef NDEBUG
        debug_draw_bounds = (std::getenv("BTK_DRAW_BOUNDS") != nullptr);
        #endif

        #ifdef BTK_RT_FPS
        set_rt_fps(BTK_RT_FPS);
        #endif
    }
    Window::~Window(){
        //Unregister window
        if(registered){
            GetSystem()->unregister_window(this);
        }
        //Stop timer if
        if(rt_draw_timer != 0){
            SDL_RemoveTimer(rt_draw_timer);
        }

        //Delete widgets
        clear_childrens();
        SDL_FreeCursor(cursor);
        //destroy the render before destroy the window
        delete _render;
        //destroy Device
        delete _device;

        SDL_DestroyWindow(win);
    }
    //Draw window
    void Window::draw(Renderer &render,Uint32 timestamp){
        
        BTK_LOGINFO("[Window]Draw %d",winid);
        
        std::lock_guard<std::recursive_mutex> locker(mtx);
        render.begin();
        render.clear(bg_color);
        //Draw each widget
        Group::draw(render,timestamp);
        //Flush to screen
        render.end();
    }
    void Window::redraw(){
        if(not visible()){
            //Window is not visible
            return;
        }
        auto current = SDL_GetTicks();
        if(fps_limit > 0){
            //Has limit
            Uint32 durl = 1000 / fps_limit;
            if(current - last_redraw_ticks < durl and draw_event_counter != 0){
                //Too fast,ignore it
                // BTK_LOGINFO("[Window] %d : Drawing too fast,ignored timestamp %d",winid,current);
                // BTK_LOGINFO("[Window] %d : Drawing pending %d",winid,draw_event_counter);
                return;
            }
        }
        //Update it
        last_redraw_ticks = current;
        draw_event_counter ++;
        
        SDL_Event event;
        event.type = GetSystem()->redraw_win_ev_id;
        event.user.timestamp = current;
        event.user.windowID = id();
        SDL_PushEvent(&event);
    }
    void Window::handle_draw(Uint32 ticks){
        draw_event_counter --;
        //redraw window
        if(fps_limit > 0){
            //Check the limit
            Uint32 current = SDL_GetTicks();
            Uint32 durl = 1000 / fps_limit;
            if(ticks + durl < current and last_draw_ticks > current - durl * 2){
                //Too late(and has been drawed recently)
                //2 is a magic number(I think it is fit)
                //Drop it
                BTK_LOGINFO("[Window] %d: Too Late , Drop the frame timestamp",winid,ticks);    
                return;
            }
            last_draw_ticks = ticks;
        }
        draw(*_render,ticks);
    }
    //TryCloseWIndow
    bool Window::close(){
        if(not IsMainThread()){
        //send a close request
            SDL_Event event;
            SDL_zero(event);
            event.type = SDL_WINDOWEVENT;
            event.window.timestamp = SDL_GetTicks();
            event.window.windowID = winid;
            event.window.event = SDL_WINDOWEVENT_CLOSE;
            SDL_PushEvent(&event);
            return false;
        }
        
        bool cancel = false;
        _signal_close(cancel);
        if(cancel){
            return false;
        }
        hide();
        //Let user destroy window the window
        _signal_closed.defer_emit();
        //Remove window from system
        GetSystem()->unregister_window(this);

        return true;
    }
    //DropFilecb
    void Window::query_dpi(float *ddpi,float *hdpi,float *vdpi){
        int idx = SDL_GetWindowDisplayIndex(win);
        if(idx == -1){
            throwSDLError();
        }
        if(SDL_GetDisplayDPI(idx,ddpi,hdpi,vdpi) == -1){
            throwSDLError();
        }
    }
    // update widgets postions
    void Window::update(){
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
            if(flat_widget){
                auto [w,h] = size();
                widget->set_rect(0,0,w,h);
            }
        }
        else{
            //not to change widgets postion
            //update each Layout
            // Layout *layout;
            // for(auto widget:widgets_list){
            //     layout = dynamic_cast<Layout*>(widget);
            //     if(layout != nullptr){
            //         layout->update();
            //     }
            // }
            Event event(Event::LayoutUpdate);
            handle(event);
        }
    }
    void Window::set_rect(const Rect &r){
        Widget::set_rect(r);
        update();
        redraw();
    }
    void Window::set_rt_fps(Uint32 fps){
        fps = clamp(fps,0u,fps_limit);

        BTK_LOGINFO("[Window::set_rt_fps] Set Window %d => %d",winid,fps);
        rt_draw_fps = fps;
        if(fps == 0 and rt_draw_timer != 0){
            SDL_RemoveTimer(rt_draw_timer);
            rt_draw_timer = 0;
        }
        else if(rt_draw_timer == 0){
            //New timer
            rt_draw_timer = SDL_AddTimer(
                1000 / rt_draw_fps,
                [](Uint32 interval,void *win){
                    return static_cast<Window*>(win)->rt_timer_cb(interval);
                },this
            );
            //Check
            if(rt_draw_timer == 0){
                rt_draw_fps = 0;
                throwSDLError();
            }
        }
    }
    void Window::set_modal(bool v){
        BTK_LOGINFO("[Window::set_modal] %d => %s",winid,v?"true":"false");
        std::lock_guard locker(mtx);
        modal = v;
        //Update window attr
        if(modal){
            //Set modal save sdl_flags
            last_win_flags = SDL_GetWindowFlags(win);
            //Make it unresizable
            SDL_SetWindowResizable(win,SDL_FALSE);
        }
        else{
            //Restore sdl_flags
            //If has SDL_Window_Resizable flag,it will be set
            if(last_win_flags & SDL_WINDOW_RESIZABLE){
                SDL_SetWindowResizable(win,SDL_TRUE);
            }
        }
    }
    void Window::move(int x,int y){
        std::lock_guard locker(mtx);
        SDL_SetWindowPosition(win,x,y);
    }
    void Window::raise(){
        std::lock_guard locker(mtx);
        SDL_RaiseWindow(win);
    }
    Point Window::position() const{
        int x,y;
        SDL_GetWindowPosition(win,&x,&y);
        return Point(x,y);
    }
    Size  Window::size() const{
        int w,h;
        SDL_GetWindowSize(win,&w,&h);
        return Size(w,h);
    }
    Uint32 Window::rt_timer_cb(Uint32 last_interval){
        //May there has thread conflict
        redraw();
        return 1000 / rt_draw_fps;
    }
    Point Window::mouse_position() const{
        int wx,wy;
        int x,y;
        SDL_GetMouseState(&x,&y);
        SDL_GetWindowPosition(win,&wx,&wy);
        return Point(x - wx,y - wy);
    }
    void Window::set_fullscreen(bool val){
        Uint32 flags;
        if(val){
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        else{
            flags = 0;
        }
        if(SDL_SetWindowFullscreen(win,flags) != 0){
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
        SDL_SetWindowResizable(win,s_val);
    }
    void Window::set_boardered(bool val){
        SDL_bool s_val;
        if(val){
            s_val = SDL_TRUE;
        }
        else{
            s_val = SDL_FALSE;
        }
        SDL_SetWindowBordered(win,s_val);
    }
    //multi threading
    void Window::lock(){
        mtx.lock();
    }
    void Window::unlock(){
        mtx.unlock();
    }
    //Show window and set Widget postions
    void Window::done(){
        update();
        show();
    }
    void Window::show(){
        Widget::show();
    }
    u8string_view Window::title() const{
        return SDL_GetWindowTitle(win);
    }
}
//Event Processing
namespace Btk{
    bool Window::handle(Event &event){
        if(Widget::handle(event)){
            return true;
        }
        switch(event.type()){
            case Event::SDLWindow:{
                //Is SDL_WindowEvent
                return handle_sdl(event);
            }
            case Event::Resize:{
                auto &e = event_cast<ResizeEvent&>(event);
                SDL_SetWindowSize(win,e.w,e.h);
                return true;
            }
            case Event::Show:{
                SDL_ShowWindow(win);
                if(not registered){
                    registered = true;
                    //Register to the event loop
                    GetSystem()->register_window(this);
                }
                return true;
            }
            case Event::Hide:{
                SDL_HideWindow(win);
                return true;
            }
            default:{
                if(Group::handle(event)){
                    return true;
                }
            }
        }
        return _signal_event(event);
    }
    bool Window::handle_sdl(Event &v){
        const SDL_Event &event = *(static_cast<SDLEvent&>(v).sdl_event);
        switch(event.window.event){
            case SDL_WINDOWEVENT_EXPOSED:{
                draw_event_counter ++;
                handle_draw(event.window.timestamp);
                break;
            }
            case SDL_WINDOWEVENT_HIDDEN:{
                attr.hide = true;
                break;
            }
            case SDL_WINDOWEVENT_SHOWN:{
                attr.hide = false;
                break;
            }
            case SDL_WINDOWEVENT_CLOSE:{
                //Drop event if modal
                if(modal){
                    return true;
                }
                //Close window;
                close();
                break;
            }
            case SDL_WINDOWEVENT_RESIZED:{
                //window resize
                set_rectangle(0,0,event.window.data1,event.window.data2);
                // on_resize(event.window.data1,event.window.data2);
                break;
            }
            case SDL_WINDOWEVENT_ENTER:{
                std::lock_guard<std::recursive_mutex> locker(mtx);
                if(cursor != nullptr){
                    //has windows cursor
                    //set cursor
                    SDL_SetCursor(cursor);
                }
                Event enter(Event::Enter);
                handle(enter);
                _signal_enter();
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
                Event event(Event::Leave);
                handle(event);
                _signal_leave();
                break;
            }
            case SDL_WINDOWEVENT_FOCUS_GAINED:{
                BTK_LOGINFO("[Window]Focus Gained %d",winid);
                _signal_keyboard_take_focus();
                break;
            }
            case SDL_WINDOWEVENT_FOCUS_LOST:{
                BTK_LOGINFO("[Window]Focus Lost %d",winid);
                _signal_keyboard_lost_focus();
                break;
            }
            case SDL_WINDOWEVENT_MOVED:{
                BTK_LOGINFO("[Window]Move To (%d,%d) %d",event.window.data1,event.window.data2,winid);
                _signal_moved(event.window.data1,event.window.data2);
                break;
            }
            default:{
                return false;
            }
        }
        return true;
    }
    bool Window::handle_motion(MotionEvent &event){
        //Drop event if modal
        if(modal){
            return false;
        }

        if(mouse_pressed and not drag_rejected and not dragging){
            //Try gen begin
            DragEvent drag(Event::DragBegin,event);

            if(not Group::handle_drag(drag) or drag.is_rejected()){
                //Rejected
                drag_rejected = true;
            }
            else{
                //Is dragging and the widget accepted it
                SDL_CaptureMouse(SDL_TRUE);
                dragging = true;
            }
        }
        if(dragging){
            //Is dragging
            DragEvent drag(Event::Drag,event);
            handle_drag(drag);
        }
        return Group::handle_motion(event);
    }
    bool Window::handle_mouse(MouseEvent &event){
        //Drop event if modal
        if(modal){
            return false;
        }

        if(event.state == MouseEvent::Pressed){
            mouse_pressed = true;
        }
        else{
            if(dragging){
                SDL_CaptureMouse(SDL_FALSE);
                DragEvent drag(Event::DragEnd,event.x,event.y,0,0);
                Group::handle_drag(drag);
            }
            //Clear the flags
            mouse_pressed = false;
            dragging = false;
            drag_rejected = false;
        }
        return Group::handle_mouse(event);
    }
    bool Window::handle_drop(DropEvent &event){
        //Drop event if modal
        if(modal){
            return false;
        }

        //TODO Send to current widget
        int ms_x,ms_y,win_x,win_y;
        SDL_GetGlobalMouseState(&ms_x,&ms_y);
        SDL_GetWindowPosition(win,&win_x,&win_y);

        event.x = ms_x - win_x;
        event.y = ms_y - win_y;

        Group::handle_drop(event);
        
        // if(event.type() == Event::DropFile){
        //     on_dropfile(event.text);
        // }
        return true;
    }
    void PushEvent(Event *event,Window &window,void (*deleter)(void *)){
        if(event == nullptr){
            return;
        }
        Uint32 id = window.id();
        //Push it into queue
        DeferCall([id,event,deleter](){
            //Make cleanup
            Btk_defer [event,deleter](){
                if(deleter != nullptr){
                    //Use deleter
                    deleter(event);
                }
                else{
                    delete event;
                }
            };
            auto win = GetSystem()->get_window_s(id);

            if(win != nullptr){
                win->handle(*event);
            }
        });
    }
}
namespace Btk{
    //add widget
    //update widgets postions
    //Set cursor
    // void Window::set_cursor(){
    //     SDL_FreeCursor(cursor);
    //     cursor = SDL_CreateSystemCursor(
    //         SDL_SYSTEM_CURSOR_ARROW
    //     );
    //     SDL_Window *win = SDL_GetMouseFocus();
    //     if(win != nullptr){
    //         if(winid == SDL_GetWindowID(win)){
    //             //Window has focus
    //             //reset cursor right now
    //             SDL_SetCursor(cursor);
    //         }
    //     }
    // }
    // void Window::set_cursor(const PixBuf &pixbuf,int hotx,int hoty){
    //     SDL_FreeCursor(cursor);
    //     cursor = SDL_CreateColorCursor(
    //         pixbuf.get(),hotx,hoty
    //     );
    //     SDL_Window *win = SDL_GetMouseFocus();
    //     if(win != nullptr){
    //         if(winid == SDL_GetWindowID(win)){
    //             //Window has focus
    //             //reset cursor right now
    //             SDL_SetCursor(cursor);
    //         }
    //     }
    // }
}
namespace Btk{
    Size GetScreenSize(int index){
        SDL_DisplayMode mode;
        if(SDL_GetCurrentDisplayMode(index,&mode) == -1){
            throwSDLError();
        }
        return {mode.w,mode.h};
    }
    SDL_Window *CreateWindow(u8string_view title,int w,int h,WindowFlags flags){
        auto has_flag = [](WindowFlags f1,WindowFlags f2){
            return (f1 & f2) == f2;
        };
        SDL_Window *sdl_win;
        if(has_flag(flags,WindowFlags::Transparent)){
            //Use transparent window
            //Send to platform
            #if BTK_X11 || BTK_WIN32
            sdl_win = Platform::CreateTsWindow(title,w,h,flags);
            #else
            BTK_UNIMPLEMENTED();
            #endif
        }
        else{
            //Translate flags into SDL flags
            Uint32 sdl_flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN;

            #if (BTK_X11 && defined(BTK_HAVE_OPENGL_DEVICE)) || (BTK_WIN32 && !defined(BTK_HAVE_DIRECTX_DEVICE))
            sdl_flags |= SDL_WINDOW_OPENGL;
            #endif


            if(has_flag(flags,WindowFlags::Resizeable)){
                sdl_flags |= SDL_WINDOW_RESIZABLE;
            }
            if(has_flag(flags,WindowFlags::Borderless)){
                sdl_flags |= SDL_WINDOW_BORDERLESS;
            }
            if(has_flag(flags,WindowFlags::Fullscreen)){
                sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
            }
            if(has_flag(flags,WindowFlags::OpenGL)){
                sdl_flags |= SDL_WINDOW_OPENGL;
            }
            if(has_flag(flags,WindowFlags::Vulkan)){
                sdl_flags |= SDL_WINDOW_VULKAN;
            }
            if(has_flag(flags,WindowFlags::SkipTaskBar)){
                sdl_flags |= SDL_WINDOW_SKIP_TASKBAR;
            }
            if(has_flag(flags,WindowFlags::PopupMenu)){
                sdl_flags |= SDL_WINDOW_POPUP_MENU;
            }

            //Create window
            sdl_win = SDL_CreateWindow(
                title.data(),
                SDL_WINDOWPOS_UNDEFINED,
                SDL_WINDOWPOS_UNDEFINED,
                w,h,
                sdl_flags
            );
        }
        if(sdl_win == nullptr){
            throwSDLError();
        }
        return sdl_win;
    }
    Window *GetKeyboardFocus(){
        SDL_Window *win = SDL_GetKeyboardFocus();
        if(win == nullptr){
            return nullptr;
        }
        return GetSystem()->get_window_s(SDL_GetWindowID(win));
    }
    Window *GetMouseFocus(){
        SDL_Window *win = SDL_GetMouseFocus();
        if(win == nullptr){
            return nullptr;
        }
        return GetSystem()->get_window_s(SDL_GetWindowID(win));
    }
}