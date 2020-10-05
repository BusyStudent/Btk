#include <SDL2/SDL_image.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/widget.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
namespace Btk{
    WindowImpl::WindowImpl(const char *title,int x,int y,int w,int h,int flags){
        refcount = 0;//default refcount
        //Btk::Window doesnnot has it ownship
        win = nullptr;
        render = nullptr;
        win = SDL_CreateWindow(title,x,y,w,h,flags);
        if(win == nullptr){
            //Handle err....
        }
        render = new RendererImpl(win);
        render->set_color({
            255,
            255,
            255,
            255
        });
    }
    WindowImpl::~WindowImpl(){
        //Delete widgets
        for(auto widget:widgets_list){
            delete widget;
        }
        delete render;
        SDL_DestroyWindow(win);
    }
    //Draw window
    void WindowImpl::draw(){
        #ifndef NDEBUG
        SDL_Log("[System::Renderer]Draw Window %p",win);
        #endif
        render->start();
        //Draw each widget
        for(auto widget:widgets_list){
            widget->draw();
        }
        render->done();
    }
    //TryCloseWIndow
    bool WindowImpl::on_close(){
        if(onclose_cb == nullptr){
            //default
            return true;
        }
        return onclose_cb();
    }
    //DropFilecb
    void WindowImpl::on_dropfile(std::string_view file){
        if(ondropfile_cb != nullptr){
            ondropfile_cb(file);
        }
    }
    void WindowImpl::on_resize(int,int){

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
        SDL_GetRendererOutputSize(render->render,w,h);
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
        return Main() == 0;
    }
    void Window::on_close(const OnCloseFunc &fn){
        pimpl->onclose_cb = fn;
    }
    void Window::on_dropfile(const OnDropFileFunc &fn){
        pimpl->ondropfile_cb = fn;
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
}