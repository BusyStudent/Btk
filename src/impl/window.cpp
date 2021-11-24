#include "../build.hpp"

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_timer.h>

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
    static RendererDevice *create_device(SDL_Window *_win){
        auto dev = CreateDevice(_win);
        if(dev == nullptr){
            throwRendererError("Couldnot create device");
        }
        BTK_LOGINFO("[Window::Device] Create Device %s",get_typename(dev).c_str());
        return dev;
    }
    WindowImpl::WindowImpl(SDL_Window *_win):
        win(_win),
        _device(create_device(_win)),
        render(*_device){
        //Set theme
        set_theme(Themes::GetDefault());
        //Set background color
        bg_color = theme()[Theme::Window];        
        attr.window = true;

        //Configure widget
        set_font(Font(theme().font_name(),theme().font_size()));
        //Update window rect
        rect.x = 0;
        rect.y = 0;
        SDL_GetWindowSize(win,&(rect.w),&(rect.h));
        //Set window data
        SDL_SetWindowData(win,"btk_impl",this);
        SDL_SetWindowData(win,"btk_dev",_device);
    }
    WindowImpl::~WindowImpl(){
        //Delete widgets
        clear_childrens();
        SDL_FreeCursor(cursor);
        //destroy the render before destroy the window
        render.destroy();
        //destroy Device
        delete _device;

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
        Group::draw(render);
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
}
//Event Processing
namespace Btk{
    bool WindowImpl::handle(Event &event){
        if(Group::handle(event)){
            return true;
        }
        return sig_event(event);
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
                Instance().close_window(this);
                break;
            }
            case SDL_WINDOWEVENT_RESIZED:{
                //window resize
                set_rect(0,0,event.window.data1,event.window.data2);
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
                Event event(Event::Leave);
                handle(event);
                if(not sig_leave.empty()){
                    sig_leave();
                }
                break;
            }
        }
    }
    bool WindowImpl::handle_motion(MotionEvent &event){
        if(mouse_pressed and not drag_rejected){
            //Is dragging
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
            DragEvent drag(Event::Drag,event);
            handle_drag(drag);
        }
        return Group::handle_motion(event);
    }
    bool WindowImpl::handle_mouse(MouseEvent &event){
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
    bool WindowImpl::handle_drop(DropEvent &event){
        //TODO Send to current widget
        Group::handle_drop(event);
        
        if(event.type() == Event::DropFile){
            on_dropfile(event.text);
        }
        return true;
    }
    void WindowImpl::defered_event(std::unique_ptr<Event> event){
        handle(*event);
    }
    void PushEvent(Event *event,Window &window){
        if(event == nullptr){
            return;
        }
        auto w = window.impl();
        w->defer_call(&WindowImpl::defered_event,std::unique_ptr<Event>(event));
    }
}
namespace Btk{
    Window::Window(u8string_view title,int w,int h,Flags f){
        Init();
        #ifdef __ANDROID__
        //Android need full screen
        Uint32 flags = 
            SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_OPENGL;
        #elif defined(_WIN32)
        //In Win32 we can use direct3d
        Uint32 flags = SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN;
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
        SDL_Window *sdl_window = SDL_CreateWindow(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,h,
            flags
        );
        if(sdl_window == nullptr){
            pimpl = nullptr;
            throwSDLError();
        }

        pimpl = Instance().create_window(sdl_window);
        // pimpl->container.window = pimpl;
        SDL_SetWindowData(pimpl->win,"btk_win",this);
        winid = SDL_GetWindowID(pimpl->win);
    }
    Window::Window(const NativeWindow *native_handle){
        SDL_Window *sdl_window = SDL_CreateWindowFrom(native_handle);
        if(sdl_window == nullptr){
            pimpl = nullptr;
            throwSDLError();
        }
        pimpl = Instance().create_window(sdl_window);
        // pimpl->container.window = pimpl;
        SDL_SetWindowData(pimpl->win,"btk_win",this);
        winid = SDL_GetWindowID(pimpl->win);
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
    void Window::show(){
        SDL_ShowWindow(pimpl->win);
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
    void Window::resize(int w,int h){
        SDL_SetWindowSize(pimpl->win,w,h);
    }
    
    void Window::set_icon(u8string_view file){
        return set_icon(PixBuf::FromFile(file));
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
    void Window::set_boardered(bool val){
        SDL_bool s_val;
        if(val){
            s_val = SDL_TRUE;
        }
        else{
            s_val = SDL_FALSE;
        }
        SDL_SetWindowBordered(pimpl->win,s_val);
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
    Size Window::size() const{
        Size s;
        SDL_GetWindowSize(pimpl->win,&s.w,&s.h);
        return s;
    }
    Point Window::position() const{
        Point p;
        SDL_GetWindowPosition(pimpl->win,&p.x,&p.y);
        return p;
    }
    u8string_view Window::title() const{
        return SDL_GetWindowTitle(pimpl->win);
    }
    void Window::dump_tree(FILE *output) const{
        pimpl->dump_tree(output);
    }
    Container &Window::container() const{
        return *pimpl;
    }
    void *Window::internal_data(const char *key){
        return SDL_GetWindowData(
            SDL_GetWindowFromID(winid),
            key
        );
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