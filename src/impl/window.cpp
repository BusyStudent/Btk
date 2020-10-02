#include <SDL2/SDL_image.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
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
        delete render;
        SDL_DestroyWindow(win);
    }
    //Draw window
    void WindowImpl::draw(){
        render->start();
        render->done();
    }
    //TryCloseWIndow
    bool WindowImpl::close(){
        if(onclose_cb == nullptr){
            //default
            return true;
        }
        return onclose_cb();
    }
    //DropFilecb
    void WindowImpl::dropfile(std::string_view file){
        if(ondropfile_cb != nullptr){
            ondropfile_cb(file);
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
        //send a draw request
        //README possible memory on here if window is closed
        System::instance->defer_call([](void *win){
            static_cast<WindowImpl*>(win)->draw();
        },pimpl);
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
}