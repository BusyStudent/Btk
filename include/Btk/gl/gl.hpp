#if !defined(_BTK_GL_HPP_)
#define _BTK_GL_HPP_

#include "../defs.hpp"

#ifndef __linux
    #define BTK_NEED_GLAD
#endif

#ifdef BTK_NEED_GLAD
    #include "glad.h"
#else
    #include <SDL2/SDL_opengles2.h>
#endif
namespace Btk{
namespace GL{
    void Init();
    void Quit();
    /**
     * @brief Create a framebuffer
     * 
     */
    struct BTKAPI FrameBuffer{
        /**
         * @brief Construct a new Frame Buffer object by texture
         * 
         * @param w The texture w
         * @param h The texture h
         * @param tex The texture
         * @param need_free Should we delete it ourself
         */
        FrameBuffer(int w,int h,GLuint tex,bool need_free = false);
        FrameBuffer(const FrameBuffer &) = delete;
        ~FrameBuffer();
        int w;
        int h;

        GLuint fbo;
        GLuint rbo;
        GLuint tex;

        GLuint screen_fbo;
        GLuint screen_rbo;

        bool need_free;//< Is the texture need free?
        bool ok;//< If we succeed to create it

        void bind();
        void unbind();
    };
    /**
     * @brief OpenGL Shader
     * 
     */
    struct Shader{
        Shader(GLenum type){
            glCreateShader(type);
        }
        Shader(const Shader &) = delete;
        ~Shader(){
            glDeleteShader(shader);
        }
        operator GLuint() const noexcept{
            return shader;
        }
        GLuint shader;
    };
    struct Program{
        Program(){
            program = glCreateProgram();
        }
        Program(const Program &) = delete;
        ~Program(){
            glDeleteProgram(program);
        }
        void link(){
            glLinkProgram(program);
        }
        operator GLuint() const noexcept{
            return program;
        }
        GLuint program;
    };
}
}

#endif // _BTK_GL_HPP_
