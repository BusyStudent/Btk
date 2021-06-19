#if !defined(_BTK_GL_HPP_)
#define _BTK_GL_HPP_

#include "../defs.hpp"
#include "../string.hpp"

#ifndef __linux
    #define BTK_NEED_GLAD
#endif

#ifdef BTK_NEED_GLAD
    #include "glad.h"
    #define BTK_GLAPIENTRY GLAPIENTRY
#else
    #include <SDL2/SDL_opengles2.h>
    #define BTK_GLAPIENTRY GL_APIENTRYP
#endif

/**
 * @brief OpenGL entry for function pointer
 * 
 */
#if defined(GLAPIENTRY)
    #define BTK_GLAPIENTRYP GLAPIENTRY
#elif defined(GL_APIENTRYP)
    #define BTK_GLAPIENTRYP GL_APIENTRYP
#else
    #define BTK_GLAPIENTRYP *
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
        /**
         * @brief compile code
         * 
         * @param code 
         * @return true succeed
         * @return false 
         */
        bool compile(u8string_view code){
            auto  src = reinterpret_cast<const GLchar*>(code.data());
            GLint len = code.size() * sizeof(GLchar);
            glShaderSource(shader,1,&src,&len);
            glCompileShader(shader);

            GLint status;
            glGetShaderiv(shader,GL_COMPILE_STATUS,&status);
            
            return status == GL_TRUE ? true : false;
        }
        /**
         * @brief Get Shader Info log
         * 
         * @return u8string 
         */
        u8string infolog() const{
            u8string msg;
            GLint len;
            
            glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&len);
            msg.resize(len);
            glGetShaderInfoLog(shader,msg.size(),nullptr,reinterpret_cast<GLchar*>(msg.data()));
            msg.shrink_to_fit();
            return msg;
        }
        /**
         * @brief Get the shader's source code
         * 
         * @return u8string 
         */
        u8string source() const{
            u8string code;
            GLint len;
            
            glGetShaderiv(shader,GL_SHADER_SOURCE_LENGTH,&len);
            code.resize(len);
            glGetShaderSource(shader,code.size(),nullptr,reinterpret_cast<GLchar*>(code.data()));
            code.shrink_to_fit();
            return code;
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
        void use(){
            GLint pr;
            glGetIntegerv(GL_CURRENT_PROGRAM,&pr);
            prev = pr;
            glUseProgram(program);
        }
        void unuse(){
            glUseProgram(prev);
        }
        operator GLuint() const noexcept{
            return program;
        }
        GLuint prev;
        GLuint program;
    };
    inline GLuint CurrentFBO(){
        GLint fbo;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
        return fbo;
    }
    inline GLuint CurrentRBO(){
        GLint rbo;
        glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo);
        return rbo;
    }
    inline GLuint CurrentTex(GLenum tex = GL_TEXTURE_2D){
        GLint t;
        glGetIntegerv(tex,&t);
        return t;
    }
}
}

#endif // _BTK_GL_HPP_
