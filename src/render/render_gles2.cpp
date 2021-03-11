#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/render.hpp>
#include <Btk/gl/gl.hpp>
extern "C"{
    #define NANOVG_GLES2_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_gl.h"
}
namespace Btk{
    //Create OpenGLES2 NanoVG Context
    Renderer::Renderer(SDL_Window *win){
        window = win;
        device = SDL_GL_CreateContext(win);
        if(device == nullptr){
            throw RendererError(SDL_GetError());
        }
        SDL_GL_MakeCurrent(win,device);
        #ifdef BTK_NEED_GLAD
        //Init glad
        static bool glad_wasinit = false;
        if(not glad_wasinit){
            if(BtkLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))){
                glad_wasinit = true;
            }
        }
        #endif
        nvg_ctxt = nvgCreateGLES2(
            NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG
        );
    }
    Renderer::~Renderer(){
        destroy();
    }
    //Begin rendering
    void Renderer::begin(){
        SDL_GL_MakeCurrent(window,device);
        int w,h;
        int pix_w,pix_h;

        SDL_GetWindowSize(window,&w,&h);
        SDL_GL_GetDrawableSize(window,&pix_w,&pix_h);

        glViewport(0,0,w,h);

        nvgBeginFrame(nvg_ctxt,w,h,float(pix_w) / float(w));
    }
    void Renderer::destroy(){
        nvgDeleteGLES2(nvg_ctxt);
        SDL_GL_DeleteContext(device);

        nvg_ctxt = nullptr;
        device   = nullptr;
    }
    //Clear the buffer
    int  Renderer::clear(){
        glClear(GL_COLOR_BUFFER_BIT);
        return glGetError() == GL_NO_ERROR ? 0 : 1;
    }
    //...
    int  Renderer::start(Color c){

        SDL_GL_MakeCurrent(window,device);
        int w,h;
        int pix_w,pix_h;

        SDL_GetWindowSize(window,&w,&h);
        SDL_GL_GetDrawableSize(window,&pix_w,&pix_h);

        glViewport(0,0,w,h);
        glClearColor(
            1.0f / 255 * c.r,
            1.0f / 255 * c.g,
            1.0f / 255 * c.b,
            1.0f / 255 * c.a
        );
        glClear(GL_COLOR_BUFFER_BIT);
        nvgBeginFrame(nvg_ctxt,w,h,float(pix_w) / float(w));

        return 0;
    }
    void Renderer::swap_buffer(){
        SDL_GL_SwapWindow(window);
    }
    //Delete texture
    void Renderer::free_texture(int image){
        SDL_GLContext cur = SDL_GL_GetCurrentContext();
        if(cur == device){
            //Is current context we use
            nvgDeleteImage(nvg_ctxt,image);
            return;
        }
        else{
            //Get current active Window
            SDL_Window *cur_win = SDL_GL_GetCurrentWindow();
            SDL_GL_MakeCurrent(window,device);
            nvgDeleteImage(nvg_ctxt,image);
            //Reset it
            SDL_GL_MakeCurrent(cur_win,cur);
        }
    }
    int  Renderer::set_viewport(const Rect *r){
        Rect rect;
        if(r == nullptr){
            rect.x = 0;
            rect.y = 0;
            SDL_GetWindowSize(window,&rect.w,&rect.h);
        }
        else if(r->empty()){
            rect.x = 0;
            rect.y = 0;
            SDL_GetWindowSize(window,&rect.w,&rect.h);
        }
        else{
            rect = *r;
        }
        glViewport(rect.x,rect.y,rect.w,rect.h);
        return glGetError() == GL_NO_ERROR ? 0 : 1;
    }
    Rect Renderer::get_viewport(){
        return viewport;
    }
}