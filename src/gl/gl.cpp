#include "../build.hpp"

#include <Btk/detail/core.hpp>
#include <Btk/gl/opengl.hpp>
#include <Btk/Btk.hpp>
#include <SDL2/SDL_video.h>

namespace Btk::GL{
    void Init(){
        #ifdef __ANDROID__
        //Tell the sdl,We want to use OPENGLES
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_ES | SDL_GL_CONTEXT_PROFILE_CORE);
        #else
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE);
        #endif

        #ifdef BTK_USE_GLES2
        //Use OpenGL 2.0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
        #else
        //Use OpenGL 3.0
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,0);
        #endif        
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,8);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,24);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);

        #ifndef BTK_NO_GLDEVICE
        //Register our Device
        RegisterDevice([](SDL_Window *win) -> RendererDevice*{
            //Check is OpenGL Window
            void *loader = SDL_GetWindowData(win,"btk_gl_loader");
            if(loader != nullptr){
                //Use loader
                using load_fn = GLDevice *(*)(SDL_Window *);
                auto fn = reinterpret_cast<load_fn>(loader);
                GLDevice *device = fn(win);
                return device;
            }
            else if((SDL_GetWindowFlags(win) & SDL_WINDOW_OPENGL) == SDL_WINDOW_OPENGL){
                return new GLDevice(win);
            }
            return nullptr;
        });
        #endif
    }
    void Quit(){
        SDL_GL_ResetAttributes();
    }
    void LoadLibaray(){
        static bool loaded = false;
        if(loaded){
            return;
        }
        if(SDL_GL_LoadLibrary(nullptr) == -1){
            BTK_LOGINFO("Failed to Load GLES %s",SDL_GetError());
        }
        else{
            loaded = true;
            AtExit(GL::Quit);
        }
    }
}
namespace Btk::GL{
    // //There has some code from nanovg_gl_utils.h

    // FrameBuffer::FrameBuffer(int w,int h,GLuint tex,bool f){
    //     need_free = f;
    //     this->tex = tex;
    //     this->w = w;
    //     this->h = h;
    //     //Current status
    //     GLint cur_fbo;
    //     GLint cur_rbo;

    //     glGetIntegerv(GL_FRAMEBUFFER_BINDING, &cur_fbo);
	//     glGetIntegerv(GL_RENDERBUFFER_BINDING, &cur_rbo);

    //     screen_fbo = cur_fbo;
    //     screen_rbo = cur_rbo;
    //     //Create ours fbo and rbo
    //     glGenFramebuffers(1,&fbo);
    //     glGenRenderbuffers(1,&rbo);
    //     //Bind it
    //     bind();
    //     //Code from nanovg_gl_utils.h
	//     // combine all

	//     glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, w, h);
	//     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
	//     glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
        
    //     if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

    //     #ifdef GL_DEPTH24_STENCIL8
    //         // If GL_STENCIL_INDEX8 is not supported, try GL_DEPTH24_STENCIL8 as a fallback.
    //         // Some graphics cards require a depth buffer along with a stencil.
    //         glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    //         glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    //         glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //         if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //     #endif // GL_DEPTH24_STENCIL8
    //             ok = false;
    //     }
    //     ok = true;

    //     unbind();
    // }
    // FrameBuffer::~FrameBuffer(){
    //     if(fbo != 0){
    //         glDeleteFramebuffers(1,&fbo);
    //     }
    //     if(rbo != 0){
    //         glDeleteFramebuffers(1,&rbo);
    //     }
    //     if(need_free){
    //         glDeleteTextures(1,&tex);
    //     }
    // }
    // //Reset the prev
    // void FrameBuffer::unbind(){
    //     glBindFramebuffer(GL_FRAMEBUFFER,screen_fbo);
    //     glBindRenderbuffer(GL_RENDERBUFFER,screen_rbo);
    // }
    // void FrameBuffer::bind(){
    //     glBindFramebuffer(GL_FRAMEBUFFER,fbo);
    //     glBindRenderbuffer(GL_RENDERBUFFER,rbo);
    // }
}