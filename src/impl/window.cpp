#include <SDL2/SDL_image.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
#include <Btk/widget.hpp>
#include <Btk/layout.hpp>
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
            throwSDLError();
        }
        bg_color = {
            255,
            255,
            255,
            255
        };
        cursor = nullptr;
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
        #ifndef NDEBUG
        SDL_Log("[System::Renderer]Draw Window %p",win);
        #endif
        std::lock_guard<std::recursive_mutex> locker(mtx);
        render.start(bg_color);
        //Draw each widget
        for(auto widget:widgets_list){
            //check widgets
            if((not widget->is_hided) and not(widget->pos.empty())){
                widget->draw(render);
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
            widget->pos.x = 0;
            widget->pos.y = 0;
            
            pixels_size(
                &(widget->pos.w),
                &(widget->pos.h)
            );
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
    void WindowImpl::handle_windowev(const SDL_Event &event){
        switch(event.window.event){
            case SDL_WINDOWEVENT_EXPOSED:{
                //redraw window
                draw();
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
    void WindowImpl::handle_mousemotion(const SDL_Event&){

    }
}
namespace Btk{
    Window::Window(std::string_view title,int w,int h){
        Init();
        pimpl = new WindowImpl(
            title.data(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            w,
            h,
            SDL_WINDOW_ALLOW_HIGHDPI
        );
        System::instance->register_window(pimpl);
    }
    bool Window::mainloop(){
        return Btk::run() == 0;
    }
    Window::SignalClose &Window::sig_close(){
        return pimpl->sig_close;
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
       SDL_Event event;
       SDL_zero(event);
       event.type = SDL_WINDOWEVENT;
       event.window.timestamp = SDL_GetTicks();
       event.window.windowID = SDL_GetWindowID(pimpl->win);
       event.window.event = SDL_WINDOWEVENT_EXPOSED;
       SDL_PushEvent(&event);
    }
    void Window::close(){
        //send a close request
        SDL_Event event;
        SDL_zero(event);
        event.type = SDL_WINDOWEVENT;
        event.window.timestamp = SDL_GetTicks();
        event.window.windowID = SDL_GetWindowID(pimpl->win);
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
    void Window::set_icon(const Surface &surf){
        SDL_SetWindowIcon(pimpl->win,surf.get());
    }

    Surface Window::surface(){
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
            if(SDL_GetWindowID(pimpl->win) == SDL_GetWindowID(win)){
                //Window has focus
                //reset cursor right now
                SDL_SetCursor(pimpl->cursor);
            }
        }
    }
    void Window::set_cursor(const Surface &surf,int hotx,int hoty){
        SDL_FreeCursor(pimpl->cursor);
        pimpl->cursor = SDL_CreateColorCursor(
            surf.get(),hotx,hoty
        );
        SDL_Window *win = SDL_GetMouseFocus();
        if(win != nullptr){
            if(SDL_GetWindowID(pimpl->win) == SDL_GetWindowID(win)){
                //Window has focus
                //reset cursor right now
                SDL_SetCursor(pimpl->cursor);
            }
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
}