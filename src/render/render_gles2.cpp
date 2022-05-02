#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/graphics/opengl.hpp>
#include <Btk/exception.hpp>
#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include <Btk/render.hpp>
#include <Btk/font.hpp>

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>

#include "../libs/nanovg.h"

#ifndef BTK_USE_OPENGLES2
    #define BTK_USE_OPENGLES2 0
    #define NANOVG_GLES3_IMPLEMENTATION
#else
    #undef BTK_USE_OPENGLES2
    #define BTK_USE_OPENGLES2 1
    #define NANOVG_GLES2_IMPLEMENTATION
#endif

namespace{
    #include "../libs/nanovg_gl.h"
}

//Active OpenGL Macro
//Active it
#define BTK_GL_BEGIN() \
    Btk::GL::GLGuard _glguard(this);
//Reset to prev context
#define BTK_GL_END() 

#define BTK_GL_BEGIN_SCOPE() {BTK_GL_BEGIN()
#define BTK_GL_END_SCOPE() BTK_GL_END()}

#ifndef NDEBUG

#define BTK_GL_CHECK() _btk_glcheck(glGetError(),__FILE__,__LINE__,BTK_FUNCTION)
static bool _btk_glcheck(GLenum code,const char *f,int line,const char *fn){
    if(code == GL_NO_ERROR){
        return true;
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
    return false;
}

#else
#define BTK_GL_CHECK() true
#endif
namespace Btk::GL{
    /**
     * @brief Helper for restore the device data
     * 
     */
    struct BTKHIDDEN GLGuard{
        GLGuard(GLDevice *device){
            dev = device;
            dev->gl_begin();
        }
        GLGuard(const GLGuard &) = delete;
        ~GLGuard(){
            dev->gl_end();
        }
        GLDevice *dev;
    };
}

namespace{
    struct BTKHIDDEN SDLGL:public Btk::GLAdapter{
        //Env
        ~SDLGL(){
            SDL_GL_DeleteContext(_context);
        }

        void   initialize(void *win_handle) override{
            Btk::GL::LoadLibaray();
            _window = static_cast<SDL_Window*>(win_handle);
            _context = SDL_GL_CreateContext(_window);

            if(_context == nullptr){
                Btk::throwSDLError();
            }
        }
        void   begin_context() override{
            _cur_ctxt = SDL_GL_GetCurrentContext();
            if(_cur_ctxt == _context){
                //We donnot setup The context
                _cur_win = nullptr;
            }
            else{
                _cur_win = SDL_GL_GetCurrentWindow();
                //Setup
                SDL_GL_MakeCurrent(_window,_context);
            }
        }
        void  end_context() override{
            //We need to reset to prev
            if(_cur_win != nullptr){
                SDL_GL_MakeCurrent(_cur_win,_cur_ctxt);
                _cur_win = nullptr;
            }
        }
        void   make_current() override{
            if(SDL_GL_GetCurrentContext() == _context){
                return;
            }
            SDL_GL_MakeCurrent(_window,_context);
        }

        void  *get_proc(const char *name) override{
            return SDL_GL_GetProcAddress(name);
        }
        void   get_drawable(int *w,int *h) override{
            return SDL_GL_GetDrawableSize(_window,w,h);
        }
        void   get_window_size(int *w,int *h) override{
            return SDL_GetWindowSize(_window,w,h);
        }
        bool   has_extension(const char *extname) override{
            return SDL_GL_ExtensionSupported(extname);
        }
        void   swap_buffer() override{
            return SDL_GL_SwapWindow(_window);
        }

        //OpenGL Window and Context
        SDL_Window    *_window = nullptr;
        SDL_GLContext  _context = nullptr;
        //Var for enter_context and leave_context
        SDL_Window    *_cur_win = nullptr;
        SDL_GLContext  _cur_ctxt = nullptr;
    };
}

namespace Btk{
    /**
     * @brief The renderer device impl
     * 
     */
    // struct BTKHIDDEN RendererDevice{
    //     RendererDevice(SDL_Window *win);
    //     RendererDevice(const RendererDevice &) = delete;
    //     ~RendererDevice();
    //     SDL_GLContext glctxt;//< OpenGL Context
    //     SDL_Window *window;
        
    //     GLuint screen_fbo;//< The fbo to the screen
    //     GLuint screen_rbo;//< The rbo to the screen    
    //     //TODO:Add a cache in there
    //     GL::FrameBuffer *buffer = nullptr;
    // };

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
    // void Renderer::set_texture_flags(int texture_id,TextureFlags flags){

    //     auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
    //     BTK_ASSERT(texture != nullptr);
    //     //Get cur texture
    //     GLint cur_tex;
    //     glGetIntegerv(GL_TEXTURE_2D,&cur_tex);
    //     //Bind the texture
    //     glBindTexture(GL_TEXTURE_2D,texture->tex);
    //     //Code from nanovg gl

    //     int w = texture->width;
    //     int h = texture->height;
    //     int imageFlags = int(flags);

    //     #ifdef NANOVG_GLES2
    //     // Check for non-power of 2.
    //     if (glnvg__nearestPow2(w) != (unsigned int)w || glnvg__nearestPow2(h) != (unsigned int)h) {
    //         // No repeat
    //         if ((imageFlags & NVG_IMAGE_REPEATX) != 0 || (imageFlags & NVG_IMAGE_REPEATY) != 0) {
    //             printf("Repeat X/Y is not supported for non power-of-two textures (%d x %d)\n", w, h);
    //             imageFlags &= ~(NVG_IMAGE_REPEATX | NVG_IMAGE_REPEATY);
    //         }
    //         // No mips.
    //         if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
    //             printf("Mip-maps is not support for non power-of-two textures (%d x %d)\n", w, h);
    //             imageFlags &= ~NVG_IMAGE_GENERATE_MIPMAPS;
    //         }
    //     }
    //     #endif

    //     if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
    //         if (imageFlags & NVG_IMAGE_NEAREST) {
    //             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //         } else {
    //             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //         }
    //     } else {
    //         if (imageFlags & NVG_IMAGE_NEAREST) {
    //             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //         } else {
    //             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //         }
    //     }

    //     if (imageFlags & NVG_IMAGE_NEAREST) {
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //     } else {
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     }

    //     if (imageFlags & NVG_IMAGE_REPEATX)
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    //     else
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    //     if (imageFlags & NVG_IMAGE_REPEATY)
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    //     else
    //         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
    //     texture->flags = imageFlags;
    //     BTK_GL_CHECK();
    //     //Reset
    //     glBindTexture(GL_TEXTURE_2D,cur_tex);

    // }
}
namespace Btk{
    //CloneTexture
    // int Renderer::clone_texture(int texture_id){
    //     BTK_GL_BEGIN();
    //     auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);

    //     int w = texture->width;
    //     int h = texture->height;
        
    //     int dst_tex = nvgCreateImageRGBA(nvg_ctxt,w,h,texture->flags,nullptr);
    //     BTK_GL_CHECK();

    //     auto *dst_texture = glnvgFindTexture(nvg_ctxt,dst_tex);
    //     //GL Texture
    //     GLuint tex = dst_texture->tex;
    //     //Current texture
    //     GLint cur_tex;
    //     GLint cur_fbo;
    //     glGetIntegerv(GL_TEXTURE_2D,&cur_tex);
    //     glGetIntegerv(GL_FRAMEBUFFER_BINDING,&cur_fbo);
    //     //Create a fbo
    //     GLuint fbo;
    //     glGenFramebuffers(1,&fbo);
    //     glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    //     BTK_GL_CHECK();
    //     //Attach
    //     glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture->tex,0);

    //     BTK_GL_CHECK();
    //     //Bind
    //     glBindTexture(GL_TEXTURE_2D,tex);
    //     BTK_GL_CHECK();
    //     glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,w,h);
    //     BTK_GL_CHECK();

    //     //Reset to prev
    //     glBindTexture(GL_TEXTURE_2D,cur_tex);
    //     glBindFramebuffer(GL_FRAMEBUFFER,cur_fbo);
    //     //Delete fbo
    //     glDeleteFramebuffers(1,&fbo);

    //     BTK_GL_END();

    //     return dst_tex;
    // }
    //Dump texture
    // PixBuf Renderer::dump_texture(int texture_id){
    //     BTK_GL_BEGIN();
    //     auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
    //     int w = texture->width;
    //     int h = texture->height;
    //     PixBuf buf(w,h,SDL_PIXELFORMAT_RGBA32);

    //     //Current texture and fbo
    //     GLint cur_fbo;
    //     glGetIntegerv(GL_FRAMEBUFFER_BINDING,&cur_fbo);

    //     GLuint fbo;
    //     glGenFramebuffers(1,&fbo);
    //     glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    //     BTK_GL_CHECK();

    //     //Copy into fbo
    //     glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,texture->tex,0);
    //     BTK_GL_CHECK();
    //     //Do copy
    //     //Read pixels glGetTexImage doesnnot exists in opengles
    //     glReadPixels(0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,buf->pixels);
    //     //glGetTexImage();
        
    //     BTK_GL_CHECK();
    //     //Cleanup
    //     glBindFramebuffer(GL_FRAMEBUFFER,cur_fbo);
    //     glDeleteFramebuffers(1,&fbo);
    //     BTK_GL_END();
    //     return buf;
    // }
    // void Renderer::update_texture(int texture,const void *pixels){
    //     BTK_GL_BEGIN();
    //     nvgUpdateImage(nvg_ctxt,texture,static_cast<const Uint8*>(pixels));
    //     BTK_GL_END();
    // }
    // void Renderer::update_texture(int texture_id,const Rect &r,const void *pixels){
    //     BTK_ASSERT(not r.empty());
    //     BTK_GL_BEGIN();
    //     auto *texture = glnvgFindTexture(nvg_ctxt,texture_id);
    //     int w = texture->width;
    //     int h = texture->height;

    //     GLint cur_tex;
    //     glGetIntegerv(GL_TEXTURE_2D,&cur_tex);

    //     glBindTexture(GL_TEXTURE_2D,texture->tex);
    //     glTexSubImage2D(GL_TEXTURE_2D,0,r.x,r.y,r.w,r.h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
    //     BTK_GL_CHECK();

    //     glBindTexture(GL_TEXTURE_2D,cur_tex);
    //     BTK_GL_END();
    // }
    // //create texture from pixbuf
    // Texture Renderer::create_from(const PixBuf &pixbuf,TextureFlags flags){
    //     if(pixbuf.empty()){
    //         throw RuntimeError("The pixbuf is empty");
    //     }
    //     //Convert the pixels
    //     if(pixbuf->format->format != SDL_PIXELFORMAT_RGBA32){
    //         return create_from(pixbuf.convert(SDL_PIXELFORMAT_RGBA32));
    //     }
    //     if(SDL_MUSTLOCK(pixbuf.get())){
    //         SDL_LockSurface(pixbuf.get());
    //     }
        
    //     BTK_GL_BEGIN();
    //     int t = nvgCreateImageRGBA(
    //         nvg_ctxt,
    //         pixbuf->w,
    //         pixbuf->h,
    //         int(flags),
    //         static_cast<const Uint8*>(pixbuf->pixels)
    //     );
    //     BTK_GL_END();
        
    //     if(SDL_MUSTLOCK(pixbuf.get())){
    //         SDL_UnlockSurface(pixbuf.get());
    //     }
    //     return Texture(t,this);
    // }
    // Texture Renderer::create_from_handle(const void *p_handle,int w,int h,TextureFlags flags){
    //     if(p_handle == nullptr){
    //         return Texture();
    //     }
    //     GLuint tex;
    //     tex = *static_cast<const GLuint*>(p_handle);
    //     int t = nvglCreateImageFromHandleGLES2(nvg_ctxt,tex,w,h,int(flags));
    //     return Texture(t,this);
    // }
    // Texture Renderer::create(int w,int h,TextureFlags flags){
    //     BTK_GL_BEGIN();
    //     int texture = nvgCreateImageRGBA(
    //         nvg_ctxt,
    //         w,
    //         h,
    //         int(flags),
    //         nullptr
    //     );
    //     BTK_GL_END();
    //     return Texture(texture,this);
    // }
    // RendererBackend Renderer::backend() const{
    //     return RendererBackend::OpenGL;
    // }
    // Size Renderer::output_size(){
    //     Size size;

    //     BTK_GL_BEGIN();
    //     SDL_GL_GetDrawableSize(window,&size.w,&size.h);
    //     BTK_GL_END();
    //     return size;
    // }
    // //Target
    // void Renderer::set_target(Texture &texture){
    //     GLuint tex;
    //     texture.native_handle(&tex);
    //     auto size = texture.size();

    //     BTK_GL_BEGIN();
    //     //Flush it
    //     end_frame();
    //     //Make a framebuffer
    //     device->buffer = new GL::FrameBuffer(size.w,size.h,tex);
    //     //Bind it
    //     device->buffer->bind();
    //     //Drawing at the texture
    //     glViewport(0,0,size.w,size.h);
    //     //ClearStencil
    //     glClearStencil(0);
    //     glClear(GL_STENCIL_BUFFER_BIT);

    //     begin_frame(size.w,size.h,1);
    //     //Make it flip
    //     //Because opengl framebuffer's x,y begin at
    //     //|
    //     //|
    //     //.here
        
    //     save();
        
    //     nvgTranslate(nvg_ctxt,0,size.h);
    //     nvgScale(nvg_ctxt,1,-1);

    //     BTK_GL_END();
    // }
    // void Renderer::reset_target(){
    //     BTK_GL_BEGIN();
    //     //Flsuh the nanovg stack
    //     restore();
    //     end_frame();
    //     //Bind the the screen
    //     device->buffer->unbind();
    //     delete device->buffer;
    //     device->buffer = nullptr;
    //     //Begin the new frame
    //     begin();

    //     BTK_GL_END();
    // }
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
}
//Some opengl constants
#define GL_MULTISAMPLE_ARB 0x809D
namespace Btk{
    //glGetVersion
    //from glad
    GLVersion GLAdapter::get_version(){
            
        /* Thank you @elmindreda
        * https://github.com/elmindreda/greg/blob/master/templates/greg.c.in#L176
        * https://github.com/glfw/glfw/blob/master/src/context.c#L36
        */
        int i, major, minor;

        const char* version;
        const char* prefixes[] = {
            "OpenGL ES-CM ",
            "OpenGL ES-CL ",
            "OpenGL ES ",
            NULL
        };
        auto glGetString = (PFNGLGETSTRINGPROC)get_proc("glGetString");

        version = (const char*) glGetString(GL_VERSION);
        if (!version) return {};

        for (i = 0;  prefixes[i];  i++) {
            const size_t length = strlen(prefixes[i]);
            if (strncmp(version, prefixes[i], length) == 0) {
                version += length;
                break;
            }
        }

    /* PR #18 */
    #ifdef _MSC_VER
        sscanf_s(version, "%d.%d", &major, &minor);
    #else
        sscanf(version, "%d.%d", &major, &minor);
    #endif

        GLVersion ver;

        ver.major = major; ver.minor = minor;
        ver.es = true;
        return ver;
    }
}
//RendererDevice
namespace Btk{
    GLDevice::~GLDevice(){
        //Delete OpenGL context
        BTK_GL_BEGIN_SCOPE();
        //Clear target stack
        while(not targets_stack.empty()){
            targets_stack.top().destroy(*this);
            targets_stack.pop();
        }
        BTK_GL_END_SCOPE();
        

        if(owned_adapter){
            delete adapter;
        }
    }
    GLDevice::GLDevice(void *win_handle,GLAdapter *_adapter,bool owned){
        //Create context
        owned_adapter = owned;
        adapter = _adapter;
        adapter->initialize(win_handle);

        //Load proc
        GLES3Functions::load_proc([this](const char *name){
            return adapter->get_proc(name);
        });

        set_backend(RendererBackend::OpenGL);

        //Save env
        screen_fbo = glGetCurrentFrameBuffer();
        screen_rbo = glGetCurrentRenderBuffer();
        //Check env
        //We will make sure msaa is disable by default
        //So user can use temporarily turn off/on antialias
        has_arb_multisample = adapter->has_extension("GL_ARB_multisample");
        has_arb_copy_image  = adapter->has_extension("GL_ARB_copy_image");
        //Check is opengl es
        is_gles = adapter->get_version().es;
        //Log info
        BTK_LOGINFO("[OpenGL]has_arb_multisample %d",int(has_arb_multisample));
        //Default functions
        has_msaa_fn = [](GLES3Functions &fns){
            GLint sample_count;
            fns.glGetIntegerv(GL_SAMPLES,&sample_count);
            return sample_count != 0;
        };
        //Replace functions by ext
        if(has_arb_multisample){
            set_msaa_fn = [](GLES3Functions &fns,bool value) -> bool{
                if(value){
                    fns.glEnable(GL_MULTISAMPLE_ARB);
                }
                else{
                    fns.glDisable(GL_MULTISAMPLE_ARB);
                }
                bool v = (fns.glGetError() == GL_NO_ERROR);
                BTK_LOGINFO("[OpenGL]Set msaa(%d) => %d",int(value),int(v));
                return v;
            };
            has_msaa_fn = [](GLES3Functions &fns) -> bool{
                return fns.glIsEnabled(GL_MULTISAMPLE_ARB);
            };
        }
        //Android Is OpenGLES
        #ifndef __ANDROID__
        if(not is_gles){
            gl_get_tex_image = (GLGetTexImage)adapter->get_proc("glGetTexImage");
            BTK_LOGINFO("Founded glGetTexImage");
        }
        #endif

        if(has_msaa()){
            //Try disable it
            if(set_msaa(false)){
                BTK_LOGINFO("[GLDevice]Succeed to disable msaa");
            }
        }
    }
    GLDevice::GLDevice(SDL_Window *win):
        GLDevice(win,new SDLGL(),true){
    }
    //GL Context
    void GLDevice::make_current(){
        adapter->make_current();
    }
    void GLDevice::gl_begin(){
        adapter->begin_context();
    }
    void GLDevice::gl_end(){
        adapter->end_context();
    }
    void GLDevice::gl_enter(Context ctxt){
        gl_begin();
        end_frame(ctxt);
    }
    void GLDevice::gl_leave(Context ctxt){
        if(targets_stack.empty()){
            //in screen
        }
        else{
            //In target
            auto &target = targets_stack.top();
            begin_frame_ex(ctxt,
                target.w,
                target.h,
                1
            );
        }
        gl_end();
    }
    //Frame
    void GLDevice::begin_frame(Context ctxt,float w,float h,float ratio){
        make_current();
        nvgBeginFrame(ctxt,w,h,ratio);
    }
    void GLDevice::cancel_frame(Context ctxt){
        make_current();
        nvgCancelFrame(ctxt);
    }
    void GLDevice::end_frame(Context ctxt){
        make_current();
        nvgEndFrame(ctxt);
    }
    //Ctxt
    NVGcontext *GLDevice::create_context(){
        BTK_GL_BEGIN();
        //Check env
        GLint stencil_size;
        glGetFramebufferAttachmentParameteriv(
            GL_DRAW_FRAMEBUFFER,
            GL_STENCIL,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE,
            &stencil_size
        );
        BTK_GL_CHECK();
        int flags = 0;

        #ifndef NDEBUG
        flags |= NVG_DEBUG;
        #endif
        if(not has_msaa()){
            //No MSAA
            BTK_LOGINFO("No MSAA on create context");
            flags |= NVG_ANTIALIAS;
        }
        else{
            //HAS MSAA
            BTK_LOGINFO("Has MSAA on create context");
        }
        //Check stencil
        if(stencil_size >= 8){
            BTK_LOGINFO("Enable Stencil on create context");
            flags |= NVG_STENCIL_STROKES;
        }
        #if BTK_USE_OPENGLES2
        return nvgCreateGLES2(flags,this);
        #else
        return nvgCreateGLES3(flags,this);
        #endif
        BTK_GL_END();
    }
    void GLDevice::destroy_context(Context ctxt){
        BTK_GL_BEGIN();
        #if BTK_USE_OPENGLES2
        return nvgDeleteGLES2(ctxt);        
        #else
        return nvgDeleteGLES3(ctxt);
        #endif
        BTK_GL_END();
    }
    //Check context
    bool GLDevice::has_msaa(){
        BTK_GL_BEGIN();
        return has_msaa_fn(*this);
        BTK_GL_END();
    }
    bool GLDevice::set_msaa(bool val){
        if(set_msaa_fn == nullptr){
            //No impl,Unsupport
            return false;
        }
        BTK_GL_BEGIN();
        return set_msaa_fn(*this,val);
        BTK_GL_END();
    }
    //Output size
    bool GLDevice::output_size(Size *logical_size,Size *physical_size){
        bool val = true;
        if(logical_size != nullptr){
            adapter->get_window_size(
                &(logical_size->w),
                &(logical_size->h)
            );
        }
        if(physical_size != nullptr){
            BTK_GL_BEGIN();
            adapter->get_drawable(
                &(physical_size->w),
                &(physical_size->h)
            );
            BTK_GL_END();
        }
        return val;
    }
    //Buffer
    void GLDevice::swap_buffer(){
        adapter->swap_buffer();
    }
    void GLDevice::clear_buffer(Color c){
        BTK_GL_BEGIN();
        
        glClearStencil(0);
        glClearColor(
            1.0f / 255 * c.r,
            1.0f / 255 * c.g,
            1.0f / 255 * c.b,
            1.0f / 255 * c.a
        );
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        BTK_GL_END();
    }

    template<class Callable>
    bool read_pixels_helper(Callable &&c,int w,int h,Uint32 fmt,void *pixel){
        void *target;
        if(fmt != PixelFormat::RGBA32){
            //Try to use a temp buffer
            target = SDL_malloc(w * h * SDL_BYTESPERPIXEL(Uint32(fmt)));
            if(target == nullptr){
                return false;
            }
        }
        else{
            target = pixel;
        }
        c(target);
        if(target != pixel){
            //Is tmp buffer
            int ret = SDL_ConvertPixels(
                w,
                h,
                PixelFormat::RGBA32,
                target,
                SDL_BYTESPERPIXEL(PixelFormat::RGBA32) * w,
                fmt,
                pixel,
                SDL_BYTESPERPIXEL(Uint32(fmt)) * w
            );
            SDL_free(target);
            if(ret != 0){
                return false;
            }
        }
        return true;
    }

    void GLDevice::read_pixels(Context ctxt,TextureID id,PixelFormat fmt,const Rect *r,void *pix){
        BTK_GL_BEGIN();
        if(ctxt == nullptr){
            //Read current frame buffer
            Rect rect;
            Rect screen = glGetCurrentViewPort();

            if(screen.empty()){
                //We cannot get current framebuffer size
                throwRendererError("Bad framebuffer");
            }

            if(r != nullptr){
                rect = *r;
            }
            else{
                //By screen size
                rect.x = 0;
                rect.y = 0;
                rect.w = screen.w;
                rect.h = screen.h;
            }
            //Tranlstate to OpenGL 
            rect.y = screen.h - rect.y - rect.h;
            //End
            auto callback = [&](void *pixel){
                glReadPixels(rect.x,rect.y,rect.w,rect.h,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
            };
            if(not read_pixels_helper(callback,fmt,rect.w,rect.h,pix)){
                //Error on convert
                throwSDLError();
            }
        }
        else if(gl_get_tex_image != nullptr and r == nullptr){
            //Read from texture with ext
            auto texture = glnvgFindTexture(ctxt,id);
            //Restore and bind
            auto cur_tex = glGetCurrentTexture2D();
            glBindTexture(GL_TEXTURE_2D,texture->tex);
            //Begin get
            //TODO Use helper
            auto callback = [&](void *pixel){
                gl_get_tex_image(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
            };
            bool ok = read_pixels_helper(callback,texture->width,texture->height,fmt,pix);
            glBindTexture(GL_TEXTURE_2D,cur_tex);
            if(not ok){
                throwSDLError();
            }
        }
        else{
            auto texture = glnvgFindTexture(ctxt,id);
            int w = texture->width;
            int h = texture->height;
            //Assign rect
            Rect rect;
            if(r != nullptr){
                rect = *r;
            }
            else{
                rect.x = 0;
                rect.y = 0;
                rect.w = w;
                rect.h = h;
            }
            //Tranlstate to OpenGL 
            rect.y = h - rect.y - rect.h;
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
            auto callback = [&](void *pixel){
                glReadPixels(rect.x,rect.y,rect.w,rect.h,GL_RGBA,GL_UNSIGNED_BYTE,pixel);
            };
            bool ok = read_pixels_helper(callback,rect.w,rect.h,fmt,pix);
            //glGetTexImage();
            BTK_GL_CHECK();
            //Cleanup
            glBindFramebuffer(GL_FRAMEBUFFER,cur_fbo);
            glDeleteFramebuffers(1,&fbo);
            if(not ok){
                throwSDLError();
            }
        }
        BTK_GL_END();
    }
    void GLDevice::set_viewport(const Rect *r){
        BTK_GL_BEGIN();
        Rect rect = {0,0,0,0};
        if(r == nullptr){
            adapter->get_drawable(&(rect.w),&(rect.h));
        }
        else{
            rect = *r;
        }
        glViewport(rect.x,rect.y,rect.w,rect.h);

        BTK_GL_END();
    }
    //Texture
    TextureID GLDevice::create_texture(Context ctxt,int w,int h,TextureFlags f,const void *pix){
        BTK_GL_BEGIN();
        return nvgCreateImageRGBA(ctxt,w,h,int(f),static_cast<const unsigned char*>(pix));
        BTK_GL_END();
    }
    TextureID GLDevice::create_texture(Context ctxt,GLuint tex,int w,int h,TextureFlags f,bool owned){
        if(not owned){
            f |= TextureFlags(NVG_IMAGE_NODELETE);
        }
        #if BTK_USE_OPENGLES2
        return nvglCreateImageFromHandleGLES2(ctxt,tex,w,h,int(f));
        #else
        return nvglCreateImageFromHandleGLES3(ctxt,tex,w,h,int(f));
        #endif
    }
    TextureID GLDevice::clone_texture(Context nvg_ctxt,TextureID texture_id){
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
    bool GLDevice::query_texture(Context ctxt,
                                 TextureID id,
                                 Size *size,
                                 void *nt_h,
                                 TextureFlags *f){
        auto tex = glnvgFindTexture(ctxt,id);
        if(tex == nullptr){
            return false;
        }
        //Found texture
        //Set data
        if(size != nullptr){
            size->w = tex->width;
            size->h = tex->height;
        }
        if(nt_h != nullptr){
            *static_cast<GLuint*>(nt_h) = tex->tex;
        }
        if(f != nullptr){
            *f = TextureFlags(tex->flags);
        }
        return true;
    }
    void GLDevice::destroy_texture(Context ctxt,TextureID id){
        BTK_GL_BEGIN();
        nvgDeleteImage(ctxt,id);
        BTK_GL_END();
    }
    void GLDevice::update_texture(Context ctxt,TextureID id,const Rect *r,const void *pix){
        BTK_GL_BEGIN();
        RendererDevice::update_texture(ctxt,id,r,pix);
        BTK_GL_END();
    }
    //Lock Texture
    struct GLLockHead{
        LockFlag flag;
        Rect rect;
    };
    void *GLDevice::lock_texture(
        Context ctxt,
        TextureID id,
        const Rect *r,
        LockFlag flag
    ){
        Rect rect;
        if(r == nullptr){
            rect.x = 0;
            rect.y = 0;
            auto [w,h] = texture_size(ctxt,id);
            rect.w = w;
            rect.h = h;
        }
        else{
            rect = *r;
        }
        //Alloc mem
        void *mem = SDL_malloc(
            sizeof(GLLockHead) + SDL_BYTESPERPIXEL(PixelFormat::RGBA32) * rect.w * rect.h
        );
        if(mem == nullptr){
            return nullptr;
        }
        void *pixels = static_cast<Uint8*>(mem) + sizeof(GLLockHead);
        //Do op
        static_cast<GLLockHead*>(mem)->flag = flag;
        static_cast<GLLockHead*>(mem)->rect = rect;
        if((flag & Read) == Read){
            read_pixels(
                ctxt,
                id,
                PixelFormat::RGBA32,
                &rect,
                pixels
            );
        }
        return pixels;
    }
    void GLDevice::unlock_texture(
        Context ctxt,
        TextureID id,
        void *pixels
    ){
        if(pixels == nullptr){
            return;
        }
        GLLockHead *data = reinterpret_cast<GLLockHead*>(
            static_cast<Uint8*>(pixels) - sizeof(GLLockHead)
        );
        if((data->flag & Write) == Write){
            update_texture(ctxt,id,&(data->rect),pixels);
        }
        SDL_free(data);
    }
    //Set flags
    bool GLDevice::configure_texture(
        Context ctxt,
        TextureID id,
        const TextureFlags *flag){
        
        if(flag != nullptr){
            auto tex = glnvgFindTexture(ctxt,id);
            if(tex == nullptr){
                return false;
            }
            int imageFlags = int(*flag);

            GLuint cur_tex = glGetCurrentTexture2D();
            
            //Code from nanovg_gl.h
        #ifdef NANOVG_GLES2
            // Check for non-power of 2.
            int w = tex->width;
            int h = tex->height;
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

        #if defined (NANOVG_GL2)
            // GL 1.4 and later has support for generating mipmaps using a tex parameter.
            if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
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
            // The new way to build mipmaps on GLES and GL3
        #if !defined(NANOVG_GL2)
            if (imageFlags & NVG_IMAGE_GENERATE_MIPMAPS) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }
        #endif
            bool val = (glGetError() == GL_NO_ERROR);
            //Reset to prev
            glBindTexture(GL_TEXTURE_2D,cur_tex);
            if(val){
                //Succeed ,update the flags
                tex->flags = imageFlags;
            }
            return val;
        }
        else{
            return false;
        }
    }
    //Target
    void GLDevice::set_target(Context ctxt,TextureID id){
        BTK_GL_BEGIN();
        //Add new target
        auto [w,h] = texture_size(ctxt,id);
        targets_stack.emplace(*this,w,h,id);
        //Check is ok
        if(not targets_stack.top().ok){
            targets_stack.top().destroy(*this);
            targets_stack.pop();
            throwRendererError("Failed to set fbo");
        }
        //End current frame
        end_frame(ctxt);
        //Bind it
        targets_stack.top().bind(*this);
        //Set Viewport
        glViewport(0,0,w,h);
        //Begin the frame at the framebuffer
        begin_frame_ex(ctxt,w,h,1);
        // Make it flip
        // Because opengl framebuffer's x,y begin at
        // |
        // |
        // .here
        nvgSave(ctxt);
        nvgTranslate(ctxt,0,h);
        nvgScale(ctxt,1,-1);

        BTK_GL_END();
    }
    void GLDevice::reset_target(Context ctxt){
        BTK_GL_BEGIN();
        if(not targets_stack.empty()){
            //End current frame
            end_frame(ctxt);
            //Reset the flip state
            nvgRestore(ctxt);

            targets_stack.top().unbind(*this);
            targets_stack.top().destroy(*this);
            targets_stack.pop();
            if(targets_stack.empty()){
                //Reset to screen
                auto phy = physical_size();
                auto log = logical_size();
                glViewport(0,0,log.w,log.h);
                begin_frame_ex(ctxt,log.w,log.h,phy.w / log.w);
            }
            else{
                //Reset to prev texture
                GLTarget &tag = targets_stack.top();
                glViewport(0,0,tag.w,tag.h);
                begin_frame_ex(ctxt,tag.w,tag.h,1);
                // Make it flip
                // Because opengl framebuffer's x,y begin at
                // |
                // |
                // .here
                nvgSave(ctxt);
                nvgTranslate(ctxt,0,tag.h);
                nvgScale(ctxt,1,-1);
                
            }
        }
        BTK_GL_END();
    }
    inline
    GLTarget::GLTarget(GLES3Functions &fns,int w,int h,GLuint tex){
        this->tex = tex;
        this->w = w;
        this->h = h;

        //Save status
        prev_fbo = fns.glGetCurrentFrameBuffer();
        prev_rbo = fns.glGetCurrentRenderBuffer();
        //Create ours fbo and rbo
        fns.glGenFramebuffers(1,&fbo);
        fns.glGenRenderbuffers(1,&rbo);
        //Bind it
        bind(fns);
        //Code from nanovg_gl_utils.h
	    // combine all

	    fns.glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);
	    fns.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	    fns.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        
        if (fns.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

        #ifdef GL_DEPTH24_STENCIL8
            // If GL_STENCIL_INDEX8 is not supported, try GL_DEPTH24_STENCIL8 as a fallback.
            // Some graphics cards require a depth buffer along with a stencil.
            fns.glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
            fns.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
            fns.glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

            if (fns.glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        #endif // GL_DEPTH24_STENCIL8
                ok = false;
        }
        ok = true;

        unbind(fns);
    }
    bool GLDevice::has_extension(const char *ext){
        BTK_GL_BEGIN();
        return adapter->has_extension(ext);
        BTK_GL_END();
    }
    void *GLDevice::load_proc(const char *name){
        BTK_GL_BEGIN();
        return adapter->get_proc(name);
        BTK_GL_END();
    }
}
//GLCanvas
namespace Btk{
    void GLCanvas::draw(Renderer &render){
        // //OpenGL Draw
        // //NVGcontext *ctxt = render.get();
        // //End the frame
        // //nvgEndFrame(ctxt);
        // //Restore prev ViewPort
        // if(render.backend() != RendererBackend::OpenGL){
        //     return;
        // }
        // render.end_frame();
        // //Try init 
        // if(not _is_inited){
        //     gl_init();
        //     _is_inited = true;
        // }
        // //Resetore the env
        // GLint viewport[4];
        // glGetIntegerv(GL_VIEWPORT,viewport);

        // Rect win_rect = window()->rectangle();

        // Rect area = {
        //     x(),
        //     win_rect.h - y() - h(),
        //     w(),
        //     h()
        // };

        // //Set the viewport
        // glViewport(area.x,area.y,area.w,area.h);
        // glEnable(GL_SCISSOR_TEST);
        // glScissor(area.x,area.y,area.w,area.h);


        // gl_draw();

        // //Reset the context
        // glDisable(GL_SCISSOR_TEST);

        // glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);

        // auto drawable = render.output_size();
        // //nvgBeginFrame(ctxt,drawable.w,drawable.h,render.pixels_ratio());
        // //TODO A dirty way to begin frame without clear the status 
        // GLDevice *device = static_cast<GLDevice*>(render.device());
        // device->begin_frame_ex(render.nvg_ctxt,drawable.w,drawable.h,render.pixels_ratio());
        // render.is_drawing = true;
    }
    GLDevice *GLCanvas::gl_device(){
        Renderer *render = renderer();
        if(render == nullptr){
            return nullptr;
        }
        if(render->backend() != RendererBackend::OpenGL){
            return nullptr;
        }
        return static_cast<GLDevice*>(render->device());
    }
    void GLCanvas::set_parent(Widget *w){
        if(w == nullptr and _is_inited){
            //Detach the widget
            auto dev = gl_device();
            if(dev != nullptr){
                dev->make_current();
                gl_cleanup();
            }
            _is_inited = false;
            Widget::set_parent(w);
        }
        else{
            //Set to new parent
            Widget::set_parent(w);
            auto dev = gl_device();
            if(dev != nullptr){
                //attached to the window
                //Init GL
                dev->make_current();
                gl_init();
                _is_inited = true;
            }
        }

    }
}