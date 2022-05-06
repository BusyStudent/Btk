#include "../build.hpp"

#if 0

#include <Btk/gl/opengl_adapter.hpp>
#include <SDL2/SDL_loadso.h>
#include <EGL/egl.h>

namespace{
    #define EGL_IMP_LIST \
        EGL_IMP(eglGetError) \
        EGL_IMP(eglInitialize) \
        EGL_IMP(eglChooseConfig) \
        EGL_IMP(eglCreateWindowSurface) \
        EGL_IMP(eglCreateContext) \
        EGL_IMP(eglMakeCurrent) \
        EGL_IMP(eglSwapInterval) \
        EGL_IMP(eglGetCurrentSurface) \
        EGL_IMP(eglGetProcAddress) \
        EGL_IMP(eglGetDisplay) \

    #define EGL_IMP(X) \
        decltype(::X) * X = nullptr;
    
    EGL_IMP_LIST

    #undef EGL_IMP


    EGLDisplay  egl_display;
    const char *egl_libname =
        #if BTK_LINUX
        "libEGL.so"
        #else

        #endif
    
    ;
    void *egl_lib;

    void egl_load(){
        egl_lib = SDL_LoadObject(egl_libname);
        if(egl_lib == nullptr){
            //
        }

        #define EGL_IMP(X) \
            X = reinterpret_cast<decltype(::X)*>(SDL_LoadFunction(egl_lib,#X));

        EGL_IMP_LIST

        #undef EGL_IMP
    }

    void egl_init(){
        if(egl_lib == nullptr){
            egl_load();

            egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

            
        }
    }
    void egl_cleanup() noexcept{
        SDL_UnloadObject(egl_lib);
    }
    //For EGL
    struct EGL:public Btk::GLAdapter{

        void *native_window;

    };
}

#endif