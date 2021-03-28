#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include <Btk/render.hpp>
#include <Btk/gl/gl.hpp>

#include <Btk/font.hpp>

extern "C"{
    #define NANOVG_GLES2_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_gl.h"
}

//Active OpenGL Macro
//Active it
#define BTK_GL_BEGIN() \
    SDL_GLContext _cur = SDL_GL_GetCurrentContext();\
    SDL_Window *_cur_window;\
    bool _need_reset;\
    if(_cur == device){\
        _need_reset = false;\
    }\
    else{\
        _need_reset = true; \
        _cur_window = SDL_GL_GetCurrentWindow();\
        SDL_GL_MakeCurrent(window,device);\
    }
//Reset to prev context
#define BTK_GL_END() \
    if(_need_reset){ \
      SDL_GL_MakeCurrent(_cur_window,_cur);\
    }
namespace Btk{
    void Renderer::get_texture_handle(int texture_id,void *p_handle){
        //Get GLuint from nanovg-GLES2
        auto *gl = static_cast<GLNVGcontext*>(nvgInternalParams(nvg_ctxt)->userPtr);
        //Get GLContext
        GLNVGtexture *texture = glnvg__findTexture(gl,texture_id);
        if(texture == nullptr){
            throwRendererError("Invaid texture");
        }
        if(p_handle != nullptr){
            *static_cast<GLuint*>(p_handle) = texture->tex;
        }
    }
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
        //Add default font
        int face_id = nvgCreateFont(nvg_ctxt,"",FontUtils::GetFileByName("").c_str());
        nvgFontFaceId(nvg_ctxt,face_id);
        nvgFontBlur(nvg_ctxt,0);
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
        for(auto iter = t_caches.begin();iter != t_caches.end();){
            nvgDeleteImage(nvg_ctxt,*iter);
            iter = t_caches.erase(iter);
        }
        
        nvgDeleteGLES2(nvg_ctxt);
        SDL_GL_DeleteContext(device);

        nvg_ctxt = nullptr;
        device   = nullptr;
    }
    //Clear the buffer
    void Renderer::clear(Color c){
        
        glClearStencil(0);
        glClearColor(
            1.0f / 255 * c.r,
            1.0f / 255 * c.g,
            1.0f / 255 * c.b,
            1.0f / 255 * c.a
        );

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    void Renderer::swap_buffer(){
        SDL_GL_SwapWindow(window);
    }
    //Delete texture
    void Renderer::free_texture(int texture){
        SDL_GLContext cur = SDL_GL_GetCurrentContext();
        if(cur == device){
            //Is current context we use
            nvgDeleteImage(nvg_ctxt,texture);
            return;
        }
        else{
            //Get current active Window
            SDL_Window *cur_win = SDL_GL_GetCurrentWindow();
            SDL_GL_MakeCurrent(window,device);
            nvgDeleteImage(nvg_ctxt,texture);
            //Reset it
            SDL_GL_MakeCurrent(cur_win,cur);
        }
    }
    void Renderer::update_texture(int texture,const void *pixels){
        BTK_GL_BEGIN();
        nvgUpdateImage(nvg_ctxt,texture,static_cast<const Uint8*>(pixels));
        BTK_GL_END();
    }
    //create texture from pixbuf
    Texture Renderer::create_from(const PixBuf &pixbuf){
        if(pixbuf.empty()){
            throw RuntimeError("The pixbuf is empty");
        }
        //Convert the pixels
        if(pixbuf->format->format != SDL_PIXELFORMAT_RGBA32){
            return create_from(pixbuf.convert(SDL_PIXELFORMAT_RGBA32));
        }
        if(SDL_MUSTLOCK(pixbuf.get())){
            SDL_LockSurface(pixbuf.get());
        }
        
        BTK_GL_BEGIN();
        int t = nvgCreateImageRGBA(
            nvg_ctxt,
            pixbuf->w,
            pixbuf->h,
            NVG_IMAGE_NEAREST,
            static_cast<const Uint8*>(pixbuf->pixels)
        );
        BTK_GL_END();
        
        if(SDL_MUSTLOCK(pixbuf.get())){
            SDL_UnlockSurface(pixbuf.get());
        }
        return Texture(t,this);
    }
    Texture Renderer::create(int w,int h){
        BTK_GL_BEGIN();
        int texture = nvgCreateImageRGBA(
            nvg_ctxt,
            w,
            h,
            NVG_IMAGE_NEAREST,
            nullptr
        );
        BTK_GL_END();
        return Texture(texture,this);
    }
    RendererBackend Renderer::backend() const{
        return RendererBackend::OpenGL;
    }
    Size Renderer::output_size(){
        Size size;

        BTK_GL_BEGIN();
        SDL_GL_GetDrawableSize(window,&size.w,&size.h);
        BTK_GL_END();
        return size;
    }
    #if 0
    //DumpTexture
    PixBuf Renderer::dump_texture(const Texture &texture){
        //Check the opengl context is actived
        bool is_actived;
        SDL_GLContext cur = SDL_GL_GetCurrentContext();
        SDL_Window *cur_win;
        if(cur == device){
            is_actived = true;
        }
        else{
            is_actived = false;
            SDL_GL_MakeCurrent(window,device);
            cur_win = SDL_GL_GetCurrentWindow();
        }
        //do our 


        if(not is_actived){
            //Reset to prev context
            SDL_GL_MakeCurrent(cur_win,cur);
        }
    }
    #endif
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
//GLCanvas
namespace Btk{
    void GLCanvas::draw(Renderer &render){
        //OpenGL Draw
        NVGcontext *ctxt = render.get();
        //End the frame
        nvgEndFrame(ctxt);
        //Restore prev ViewPort
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        //Set the viewport
        glViewport(rect.x,rect.y,rect.w,rect.h);
        glEnable(GL_SCISSOR_TEST);
        glScissor(rect.x,rect.y,rect.w,rect.h);


        gl_draw();

        //Reset the context
        glDisable(GL_SCISSOR_TEST);
        glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);

        auto drawable = render.output_size();
        nvgBeginFrame(ctxt,drawable.w,drawable.h,render.pixels_radio());
    }
}