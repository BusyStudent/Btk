#if !defined(_BTK_GL_HPP_)
#define _BTK_GL_HPP_

#include <stack>
#include "../defs.hpp"
#include "../string.hpp"
#include "../render.hpp"

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
    inline Rect CurrentViewPort(){
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        return Rect(viewport[0],viewport[1],viewport[2],viewport[3]);
    }
}
}

struct SDL_Window;
//GLDevice
namespace Btk{
    /**
     * @brief Output target
     * @internal It is used for GLDevice
     */
    struct BTKHIDDEN GLTarget{
        GLTarget(int w,int h,GLuint tex);
        GLTarget(const GLTarget &) = delete;
        ~GLTarget(){
            //Cleanup
            glDeleteFramebuffers(1,&fbo);
            glDeleteRenderbuffers(1,&rbo);
        }
        /**
         * @brief Reset to prev
         * 
         */
        void unbind(){
            glBindFramebuffer(GL_FRAMEBUFFER,prev_fbo);
            glBindRenderbuffer(GL_RENDERBUFFER,prev_rbo);
        }
        void bind(){
            glBindFramebuffer(GL_FRAMEBUFFER,fbo);
            glBindRenderbuffer(GL_RENDERBUFFER,rbo);
        }
        //Prev status
        GLuint prev_fbo;
        GLuint prev_rbo;
        //Current
        GLuint rbo;
        GLuint fbo;
        GLuint tex;
        //Is ok
        bool ok;
    };
    /**
     * @brief OpenGL Renderer Device
     * 
     */
    class BTKAPI GLDevice:public RendererDevice{
        public:
            GLDevice(SDL_Window *win);
            GLDevice(const GLDevice &) = delete;
            ~GLDevice();

            Context create_context() override;
            void    destroy_context(Context) override;
            
            void begin_frame(Context ctxt,
                             float w,
                             float h,
                             float pixel_ratio) override;
            void cancel_frame(Context ctxt) override;
            void end_frame(Context ctxt) override;
            //Buffer
            void clear_buffer(Color bg) override;
            void swap_buffer() override;
            void set_viewport(const Rect *r) override;

            bool output_size(
                Size *p_logical_size,
                Size *p_physical_size) override;
            //Note GLDevice only support global target
            void set_target(Context ctxt,TextureID id) override;
            void reset_target(Context ctxt) override;
            //Texture
            TextureID create_texture(Context ctxt,
                                     int w,
                                     int h,
                                     TextureFlags f,
                                     const void *pix) override;
            TextureID clone_texture(Context ctxt,TextureID) override;
            void      update_texture(Context ctxt,TextureID,const Rect *r,const void *pix) override;
            void      destroy_texture(Context ctxt,TextureID id) override;
            bool      query_texture(Context ctxt,
                                    TextureID id,
                                    Size *size,
                                    void *nt_h,
                                    TextureFlags *f) override;
            //Target
        public:
            //OpenGL operations
            void make_current();
            /**
             * @brief Save current OpenGL Context,and make self current
             * 
             */
            void gl_begin();
            /**
             * @brief Reset to the prev OpenGL context
             * 
             */
            void gl_end();
            /**
             * @brief Get current viewport
             * 
             * @return Rect 
             */
            Rect viewport(){
                make_current();
                return GL::CurrentViewPort();
            }
        private:
            //OpenGL Window and Context
            SDL_Window *_window;
            void *_context;
            //Var for gl_begin and gl_end
            SDL_Window *_cur_win;
            void *_cur_ctxt;
            //Env
            GLuint screen_fbo;
            GLuint screen_rbo;
        private:
            std::stack<GLTarget> targets_stack;
    };
}

#endif // _BTK_GL_HPP_
