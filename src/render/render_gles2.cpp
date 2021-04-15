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
    Btk::GL::GLGuard _glguard(device->glctxt,window);
//Reset to prev context
#define BTK_GL_END() 

#ifndef NDEBUG

#define BTK_GL_CHECK() _btk_glcheck(__FILE__,__LINE__,BTK_FUNCTION);
static void _btk_glcheck(const char *f,int line,const char *fn){
    GLenum code = glGetError();
    if(code == GL_NO_ERROR){
        return;
    }
    const char *errstring;
    switch(code){
        case GL_INVALID_ENUM:                  errstring = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                 errstring = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:             errstring = "INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY:                 errstring = "OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: errstring = "INVALID_FRAMEBUFFER_OPERATION"; break;
        default:                               errstring = "UNKNOWN"; break;
    }
    _Btk_Backtrace();
    fprintf(stderr,"GLError at %s:%d '%s'=> %s\n",f,line,fn,errstring);
    fflush(stderr);
}

#else
#define BTK_GL_CHECK()
#endif
namespace Btk::GL{
    /**
     * @brief Helper for restore the device data
     * 
     */
    struct BTKHIDDEN GLGuard{
        GLGuard(SDL_GLContext ctxt,SDL_Window *win){
            this->ctxt = ctxt;
            this->win = win;

            //Get current ctxt
            cur_ctxt = SDL_GL_GetCurrentContext();

            if(cur_ctxt == ctxt){
                //We donnot need set the ctxt
                cur_win = nullptr;
            }
            else{
                cur_win = SDL_GL_GetCurrentWindow();
                SDL_GL_MakeCurrent(win,ctxt);
            }
        }
        GLGuard(const GLGuard &) = delete;
        ~GLGuard(){
            if(cur_win != nullptr){
                SDL_GL_MakeCurrent(cur_win,cur_ctxt);
            }
        }
        SDL_GLContext cur_ctxt;//< Current context
        SDL_GLContext ctxt;//<Context

        SDL_Window *cur_win;//< Current Window
        SDL_Window *win;//Window
    };
}
namespace Btk{
    /**
     * @brief The renderer device impl
     * 
     */
    struct BTKHIDDEN RendererDevice{
        RendererDevice(SDL_Window *win);
        RendererDevice(const RendererDevice &) = delete;
        ~RendererDevice();
        SDL_GLContext glctxt;//< OpenGL Context
        SDL_Window *window;
        
        GLuint screen_fbo;//< The fbo to the screen
        GLuint screen_rbo;//< The rbo to the screen    
        //TODO:Add a cache in there
        GL::FrameBuffer *buffer = nullptr;
    };

    /**
     * @brief Get Texture from nanovg
     * 
     * @param ctxt 
     * @param texture_id 
     * @return GLNVGtexture* 
     */
    static GLNVGtexture *glnvgFindTexture(NVGcontext *ctxt,int texture_id){
        auto *gl = static_cast<GLNVGcontext*>(nvgInternalParams(ctxt)->userPtr);
        //Get GLContext
        GLNVGtexture *texture = glnvg__findTexture(gl,texture_id);
        if(texture == nullptr){
            throwRendererError("Invaid texture");
        }
        return texture;
    }
    void Renderer::get_texture_handle(int texture_id,void *p_handle){
        //Get GLuint from nanovg-GLES2
        auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
        if(p_handle != nullptr){
            *static_cast<GLuint*>(p_handle) = texture->tex;
        }
    }
    TextureFlags Renderer::get_texture_flags(int texture_id){
        auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
        return static_cast<TextureFlags>(texture->flags);
    }
    void Renderer::set_texture_flags(int texture_id,TextureFlags flags){
        BTK_GL_BEGIN();
        auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
        BTK_ASSERT(texture != nullptr);
        //Get cur texture
        GLint cur_tex;
        glGetIntegerv(GL_TEXTURE_2D,&cur_tex);
        //Bind the texture
        glBindTexture(GL_TEXTURE_2D,texture->tex);
        //Code from nanovg gl

        int w = texture->width;
        int h = texture->height;
        int imageFlags = int(flags);

        #ifdef NANOVG_GLES2
        // Check for non-power of 2.
        if (glnvg__nearestPow2(w) != (unsigned int)w || glnvg__nearestPow2(h) != (unsigned int)h) {
            // No repeat
            if ((imageFlags & NVG_IMAGE_REPEATX) != 0 || (imageFlags & NVG_IMAGE_REPEATY) != 0) {
                printf("Repeat X/Y is not supported for non power-of-two textures (%d x %d)\n", w, h);
                imageFlags &= ~(NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
            }
            // No mips.
            if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
                printf("Mip-maps is not support for non power-of-two textures (%d x %d)\n", w, h);
                imageFlags &= ~NVG_IMAGE_GENERATE_MIPMAPS;
            }
        }
        #endif

        if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
            if (imageFlags & NVG_IMAGE_NEAREST) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
        } else {
            if (imageFlags & NVG_IMAGE_NEAREST) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            } else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
        }

        if (imageFlags & NVG_IMAGE_NEAREST) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }

        if (imageFlags & NVG_IMAGE_REPEATX)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        if (imageFlags & NVG_IMAGE_REPEATY)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        else
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        texture->flags = imageFlags;
        BTK_GL_CHECK();
        //Reset
        glBindTexture(GL_TEXTURE_2D,cur_tex);
        BTK_GL_END();
    }
}
namespace Btk{
    //Create OpenGLES2 NanoVG Context
    Renderer::Renderer(SDL_Window *win){
        window = win;
        device = new RendererDevice(win);

        //SDL_GL_MakeCurrent(win,device->glctxt);

        SDL_GL_SetSwapInterval(1);

        nvg_ctxt = nvgCreateGLES2(
            NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG
        );
        //Add default font
        //int face_id = nvgCreateFont(nvg_ctxt,"",FontUtils::GetFileByName("").c_str());
        //nvgFontFaceId(nvg_ctxt,face_id);
        //Using default font from cache
        nvgFontFace(nvg_ctxt,"");
        nvgFontBlur(nvg_ctxt,0);
    }
    Renderer::~Renderer(){
        destroy();
    }
    //Begin rendering
    void Renderer::begin(){
        if(not is_drawing){
            SDL_GL_MakeCurrent(window,device->glctxt);
            
            int w,h;
            int pix_w,pix_h;

            SDL_GetWindowSize(window,&w,&h);
            SDL_GL_GetDrawableSize(window,&pix_w,&pix_h);

            glViewport(0,0,w,h);

            nvgBeginFrame(nvg_ctxt,w,h,float(pix_w) / float(w));
            is_drawing = true;
        }
    }
    void Renderer::destroy(){
        for(auto iter = t_caches.begin();iter != t_caches.end();){
            nvgDeleteImage(nvg_ctxt,*iter);
            iter = t_caches.erase(iter);
        }
        
        nvgDeleteGLES2(nvg_ctxt);
        
        delete device;

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
        BTK_GL_BEGIN();

        nvgDeleteImage(nvg_ctxt,texture);

        BTK_GL_END();
    }
    //CloneTexture
    int Renderer::clone_texture(int texture_id){
        BTK_GL_BEGIN();
        auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);

        int w = texture->width;
        int h = texture->height;
        
        int dst_tex = nvgCreateImageRGBA(nvg_ctxt,w,h,texture->flags,nullptr);
        BTK_GL_CHECK();

        auto *dst_texture = glnvgFindTexture(nvg_ctxt,dst_tex);
        //GL Texture
        GLuint tex = dst_texture->tex;
        //Current texture
        GLint cur_tex;
        GLint cur_fbo;
        glGetIntegerv(GL_TEXTURE_2D,&cur_tex);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING,&cur_fbo);
        //Create a fbo
        GLuint fbo;
        glGenFramebuffers(1,&fbo);
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        BTK_GL_CHECK();
        //Attach
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture->tex,0);

        BTK_GL_CHECK();
        //Bind
        glBindTexture(GL_TEXTURE_2D,tex);
        BTK_GL_CHECK();
        glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,w,h);
        BTK_GL_CHECK();

        //Reset to prev
        glBindTexture(GL_TEXTURE_2D,cur_tex);
        glBindFramebuffer(GL_FRAMEBUFFER,cur_fbo);
        //Delete fbo
        glDeleteFramebuffers(1,&fbo);

        BTK_GL_END();

        return dst_tex;
    }
    //Dump texture
    PixBuf Renderer::dump_texture(int texture_id){
        BTK_GL_BEGIN();
        auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
        int w = texture->width;
        int h = texture->height;
        PixBuf buf(w,h,SDL_PIXELFORMAT_RGBA32);

        //Current texture and fbo
        GLint cur_fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING,&cur_fbo);

        GLuint fbo;
        glGenFramebuffers(1,&fbo);
        glBindFramebuffer(GL_FRAMEBUFFER,fbo);
        BTK_GL_CHECK();

        //Copy into fbo
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture->tex,0);
        BTK_GL_CHECK();
        //Do copy
        //Read pixels glGetTexImage doesnnot exists in opengles
        glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,buf->pixels);
        //glGetTexImage();
        
        BTK_GL_CHECK();
        //Cleanup
        glBindFramebuffer(GL_FRAMEBUFFER,cur_fbo);
        glDeleteFramebuffers(1,&fbo);
        BTK_GL_END();
        return buf;
    }
    void Renderer::update_texture(int texture,const void *pixels){
        BTK_GL_BEGIN();
        nvgUpdateImage(nvg_ctxt,texture,static_cast<const Uint8*>(pixels));
        BTK_GL_END();
    }
    //create texture from pixbuf
    Texture Renderer::create_from(const PixBuf &pixbuf,TextureFlags flags){
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
            int(flags),
            static_cast<const Uint8*>(pixbuf->pixels)
        );
        BTK_GL_END();
        
        if(SDL_MUSTLOCK(pixbuf.get())){
            SDL_UnlockSurface(pixbuf.get());
        }
        return Texture(t,this);
    }
    Texture Renderer::create(int w,int h,TextureFlags flags){
        BTK_GL_BEGIN();
        int texture = nvgCreateImageRGBA(
            nvg_ctxt,
            w,
            h,
            int(flags),
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
    //Target
    void Renderer::set_target(Texture &texture){
        GLuint tex;
        texture.native_handle(&tex);
        auto size = texture.size();

        BTK_GL_BEGIN();
        //Flush it
        end_frame();
        //Make a framebuffer
        device->buffer = new GL::FrameBuffer(size.w,size.h,tex);
        //Bind it
        device->buffer->bind();
        //Drawing at the texture
        glViewport(0,0,size.w,size.h);
        //ClearStencil
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);

        begin_frame(size.w,size.h,1);
        //Make it flip
        //Because opengl framebuffer's x,y begin at
        //|
        //|
        //.here
        
        save();
        
        nvgTranslate(nvg_ctxt,0,size.h);
        nvgScale(nvg_ctxt,1,-1);

        BTK_GL_END();
    }
    void Renderer::reset_target(){
        BTK_GL_BEGIN();
        //Flsuh the nanovg stack
        restore();
        end_frame();
        //Bind the the screen
        device->buffer->unbind();
        delete device->buffer;
        device->buffer = nullptr;
        //Begin the new frame
        begin();

        BTK_GL_END();
    }
    void Renderer::make_current(){
        SDL_GL_MakeCurrent(window,device->glctxt);
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
//RendererDevice
namespace Btk{
    RendererDevice::RendererDevice(SDL_Window *win){
        glctxt = SDL_GL_CreateContext(win);
        window = win;
        
        //Failed to create
        if(glctxt == nullptr){
            throwSDLError();
        }
        SDL_GL_MakeCurrent(window,glctxt);

        #ifdef BTK_NEED_GLAD
        //Init glad
        static bool glad_wasinit = false;
        if(not glad_wasinit){
            if(BtkLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress))){
                glad_wasinit = true;
            }
        }
        #endif

        //Get fbo and rbo

        GLint fbo,rbo;

        glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
        glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo);

        screen_fbo = fbo;
        screen_rbo = rbo;

        BTK_LOGINFO("Create OpenGL Device version:%s",glGetString(GL_VERSION));
        //Makeframebuffer
    }
    RendererDevice::~RendererDevice(){
        if(glctxt != nullptr){
            delete buffer;
            SDL_GL_DeleteContext(glctxt);
        }
    }
}
//GLCanvas
namespace Btk{
    void GLCanvas::draw(Renderer &render){
        //OpenGL Draw
        //NVGcontext *ctxt = render.get();
        //End the frame
        //nvgEndFrame(ctxt);
        //Restore prev ViewPort
        render.end_frame();
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
        //nvgBeginFrame(ctxt,drawable.w,drawable.h,render.pixels_ratio());
        render.begin_frame(drawable.w,drawable.h,render.pixels_ratio());
    }
}