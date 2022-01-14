#if !defined(_BTK_GL_HPP_)
#define _BTK_GL_HPP_

#include <stack>
#include "../defs.hpp"
#include "../string.hpp"
#include "../render.hpp"
//Import PFNS and macro
#include "opengl_macro.h"

/**
 * @brief OpenGL entry for function pointer
 * 
 */
#if defined(GLAPIENTRY)
    #define BTK_GLAPIENTRYP GLAPIENTRY *
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
     * @brief dymaic Load OpenGL Library
     * 
     * @return BTKAPI 
     */
    BTKAPI void LoadLibaray();
    // /**
    //  * @brief Create a framebuffer
    //  * 
    //  */
    // struct BTKAPI FrameBuffer{
    //     /**
    //      * @brief Construct a new Frame Buffer object by texture
    //      * 
    //      * @param w The texture w
    //      * @param h The texture h
    //      * @param tex The texture
    //      * @param need_free Should we delete it ourself
    //      */
    //     FrameBuffer(int w,int h,GLuint tex,bool need_free = false);
    //     FrameBuffer(const FrameBuffer &) = delete;
    //     ~FrameBuffer();
    //     int w;
    //     int h;

    //     GLuint fbo;
    //     GLuint rbo;
    //     GLuint tex;

    //     GLuint screen_fbo;
    //     GLuint screen_rbo;

    //     bool need_free;//< Is the texture need free?
    //     bool ok;//< If we succeed to create it

    //     void bind();
    //     void unbind();
    // };
    // /**
    //  * @brief OpenGL Shader
    //  * 
    //  */
    // struct Shader{
    //     Shader(GLenum type){
    //         shader = glCreateShader(type);
    //     }
    //     Shader(const Shader &) = delete;
    //     ~Shader(){
    //         glDeleteShader(shader);
    //     }
    //     /**
    //      * @brief compile code
    //      * 
    //      * @param code 
    //      * @return true succeed
    //      * @return false 
    //      */
    //     bool compile(u8string_view code){
    //         auto  src = reinterpret_cast<const GLchar*>(code.data());
    //         GLint len = code.size() * sizeof(GLchar);
    //         glShaderSource(shader,1,&src,&len);
    //         glCompileShader(shader);

    //         GLint status;
    //         glGetShaderiv(shader,GL_COMPILE_STATUS,&status);
            
    //         return status == GL_TRUE ? true : false;
    //     }
    //     /**
    //      * @brief Get Shader Info log
    //      * 
    //      * @return u8string 
    //      */
    //     u8string infolog() const{
    //         u8string msg;
    //         GLint len;
            
    //         glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&len);
    //         msg.resize(len);
    //         glGetShaderInfoLog(shader,msg.size(),nullptr,reinterpret_cast<GLchar*>(msg.data()));
    //         msg.shrink_to_fit();
    //         return msg;
    //     }
    //     /**
    //      * @brief Get the shader's source code
    //      * 
    //      * @return u8string 
    //      */
    //     u8string source() const{
    //         u8string code;
    //         GLint len;
            
    //         glGetShaderiv(shader,GL_SHADER_SOURCE_LENGTH,&len);
    //         code.resize(len);
    //         glGetShaderSource(shader,code.size(),nullptr,reinterpret_cast<GLchar*>(code.data()));
    //         code.shrink_to_fit();
    //         return code;
    //     }
    //     operator GLuint() const noexcept{
    //         return shader;
    //     }
    //     GLuint shader;
    // };
    // struct Program{
    //     Program(){
    //         program = glCreateProgram();
    //     }
    //     Program(const Program &) = delete;
    //     ~Program(){
    //         glDeleteProgram(program);
    //     }
    //     void link(){
    //         glLinkProgram(program);
    //     }
    //     void use(){
    //         GLint pr;
    //         glGetIntegerv(GL_CURRENT_PROGRAM,&pr);
    //         prev = pr;
    //         glUseProgram(program);
    //     }
    //     void unuse(){
    //         glUseProgram(prev);
    //     }
    //     void attach(const Shader &shader){
    //         glAttachShader(program,shader.shader);
    //     }
    //     operator GLuint() const noexcept{
    //         return program;
    //     }
    //     GLuint prev;
    //     GLuint program;
    // };
    // inline GLuint CurrentFBO(){
    //     GLint fbo;
    //     glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
    //     return fbo;
    // }
    // inline GLuint CurrentRBO(){
    //     GLint rbo;
    //     glGetIntegerv(GL_RENDERBUFFER_BINDING,&rbo);
    //     return rbo;
    // }
    // inline GLuint CurrentTex(GLenum tex = GL_TEXTURE_2D){
    //     GLint t;
    //     glGetIntegerv(tex,&t);
    //     return t;
    // }
    // inline Rect CurrentViewPort(){
    //     GLint viewport[4];
    //     glGetIntegerv(GL_VIEWPORT,viewport);
    //     return Rect(viewport[0],viewport[1],viewport[2],viewport[3]);
    // }
}
// using GLShader = GL::Shader;
// using GLProgram = GL::Program;
}

struct SDL_Window;
//GLDevice
namespace Btk{
    //Functions
    #ifndef BTK_GL_STATIC
        #define BTK_GL_DECL(PFN,NAME) PFN NAME;
    #else
        #define BTK_GL_DECL(PFN,NAME) static constexpr PFN NAME = ::NAME;
    #endif
    struct GLVersion{
        int  major;
        int  minor;
        bool es;
    };
    /**
     * @brief OpenGLES2 functions
     * 
     */
    struct GLES2Functions{
        #define BTK_GL_PROCESS(PFN,NAME) BTK_GL_DECL(PFN,NAME)

        BTK_OPENGLES2_FUNCTIONS_PART

        #undef BTK_GL_PROCESS

        //load templates
        template<class Callable>
        void load_proc(Callable &&callable){
            #ifndef BTK_GL_STATIC
            
            #define BTK_GL_PROCESS(PFN,NAME) \
                this->NAME = reinterpret_cast<PFN>(callable(#NAME));
            
            BTK_OPENGLES2_FUNCTIONS_PART
            
            #undef BTK_GL_PROCESS
            
            #endif
        }
        //Helper
        GLuint glGetCurrentFrameBuffer() const{
            GLint fbo;
            glGetIntegerv(GL_FRAMEBUFFER_BINDING,&fbo);
            return fbo;
        }
        GLuint glGetCurrentRenderBuffer() const{
            GLint fbo;
            glGetIntegerv(GL_RENDERBUFFER_BINDING,&fbo);
            return fbo;
        }
        GLuint glGetCurrentTexture2D() const{
            GLint t;
            glGetIntegerv(GL_TEXTURE_2D,&t);
            return t;
        }
        Rect   glGetCurrentViewPort() const{
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT,viewport);
            return Rect(viewport[0],viewport[1],viewport[2],viewport[3]);
        }
    };
    struct GLES3Functions:public GLES2Functions{
        #define BTK_GL_PROCESS(PFN,NAME) BTK_GL_DECL(PFN,NAME)

        BTK_OPENGLES3_FUNCTIONS_PART

        #undef BTK_GL_PROCESS

        template<class Callable>
        void load_proc(Callable &&callable){
            #ifndef BTK_GL_STATIC
            //call parents
            GLES2Functions::load_proc(callable);

            #define BTK_GL_PROCESS(PFN,NAME) \
                this->NAME = reinterpret_cast<PFN>(callable(#NAME));
            
            BTK_OPENGLES3_FUNCTIONS_PART
            
            #undef BTK_GL_PROCESS

            #endif
        }
    };

    /**
     * @brief Output target
     * @internal It is used for GLDevice
     */
    struct BTKHIDDEN GLTarget{
        inline
        GLTarget(GLES3Functions &fns,int w,int h,GLuint tex);
        inline
        GLTarget(const GLTarget &) = default;
        //MSVC 's stack cannot use GLTarget with delete copy constructor
        //So we could only allow it to copy,destroy it by ourself
        void destroy(GLES3Functions &fns){
            //Cleanup
            fns.glDeleteFramebuffers(1,&fbo);
            fns.glDeleteRenderbuffers(1,&rbo);
        }
        /**
         * @brief Reset to prev
         * 
         */
        void unbind(GLES3Functions &fns){
            fns.glBindFramebuffer(GL_FRAMEBUFFER,prev_fbo);
            fns.glBindRenderbuffer(GL_RENDERBUFFER,prev_rbo);
        }
        void bind(GLES3Functions &fns){
            fns.glBindFramebuffer(GL_FRAMEBUFFER,fbo);
            fns.glBindRenderbuffer(GL_RENDERBUFFER,rbo);
        }
        //Prev status
        GLuint prev_fbo;
        GLuint prev_rbo;
        //Current
        GLuint rbo;
        GLuint fbo;
        GLuint tex;
        int w,h;
        //Is ok
        bool ok;
    };
    struct GLAttributes{

    };
    class GLAdapter{
        public:
            virtual ~GLAdapter(){};
            virtual void *create_context(void *win_handle) = 0;
            virtual void  destroy_context(void *gl_handle) = 0;
            //Env
            virtual bool   make_current(void *win_handle,void *gl_handle) = 0;
            virtual void  *get_current_context() = 0;
            virtual void  *get_current_window() = 0;
            virtual void  *get_proc(const char *name) = 0;
            virtual void   get_drawable(void *win_handle,int *w,int *h) = 0;
            virtual void   get_window_size(void *win_handle,int *w,int *h) = 0;
            virtual bool   has_extension(const char *extname) = 0;
            virtual void   swap_window(void *win_handle) = 0;

            BTKAPI
            GLVersion get_version();
    };
    /**
     * @brief OpenGL Renderer Device
     * 
     */
    class BTKAPI GLDevice:public RendererDevice,public GLES3Functions{
        public:
            GLDevice(SDL_Window *win);
            GLDevice(void *win_handle,GLAdapter *adapter,bool owned = false);
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
            /**
             * @brief Create a texture object from native handle
             * 
             * @param ctxt 
             * @param tex 
             * @param w 
             * @param h 
             * @param f 
             * @param owned 
             * @return TextureID 
             */
            TextureID create_texture(Context ctxt,
                                     GLuint tex,
                                     int w,
                                     int h,
                                     TextureFlags f,
                                     bool owned = true);
            TextureID clone_texture(Context ctxt,TextureID) override;
            void      update_texture(Context ctxt,TextureID,const Rect *r,const void *pix) override;
            void      destroy_texture(Context ctxt,TextureID id) override;
            bool      query_texture(Context ctxt,
                                    TextureID id,
                                    Size *size,
                                    void *nt_h,
                                    TextureFlags *f) override;
            void*     lock_texture(
                Context ctxt,
                TextureID id,
                const Rect *r,
                LockFlag flag
            ) override;
            void      unlock_texture(
                Context ctxt,
                TextureID id,
                void *pixels
            ) override;
            bool      configure_texture(
                Context ctxt,
                TextureID id,
                const TextureFlags *flag
            ) override;
            
            //Pixels
            /**
             * @brief Read Pixels
             * 
             * @param ctxt The context(nullptr on screen)
             * @param id The texture(will be ignored when ctxt = nullptr)
             */
            void read_pixels(
                Context ctxt,
                TextureID id,
                PixelFormat fmt,
                const Rect *r,
                void *pix
            );
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
            
            void gl_enter(Context ctxt);
            void gl_leave(Context ctxt);
            /**
             * @brief Enable/Disable MSAA
             * @note You would be better to check the return value
             * @param val true on enable
             * 
             * @return true on Support
             * @return false on Unsupport
             */
            [[nodiscard]]
            bool set_msaa(bool val = true);
            /**
             * @brief Check current context has msaa
             * 
             * @return true 
             * @return false 
             */
            bool has_msaa();
            /**
             * @brief Check the Env support 
             * 
             * @param ext_name 
             */
            bool  has_extension(const char *ext_name);
            /**
             * @brief Get the address object
             * 
             * @param proc The function name
             * @return void* 
             */
            void *load_proc(const char *proc);

            Rect viewport(){
                make_current();
                return glGetCurrentViewPort();
            }
        private:
            //OpenGL Window and Context
            void *_window;
            void *_context;
            //Var for gl_begin and gl_end
            void *_cur_win;
            void *_cur_ctxt;
            //Env
            GLuint screen_fbo;
            GLuint screen_rbo;
            //Enable /disable by glEnable(GL_MULTISAMPLE_ARB);
            bool has_arb_multisample = false;
            bool has_arb_copy_image  = false;
            bool is_gles = true;
            //Function ptr
            bool (*has_msaa_fn)(GLES3Functions &fns) = nullptr;//< Check has msaa
            bool (*set_msaa_fn)(GLES3Functions &fns,bool) = nullptr;//Enable / disable msaa
            //Ext
            using GLGetTexImage = void(BTK_GLAPIENTRYP )(
                GLenum target,
                GLint level,
                GLenum format,
                GLenum type,
                void * pixels);

            GLGetTexImage gl_get_tex_image = nullptr;
            //Adapter
            GLAdapter *adapter;
            bool owned_adapter;
        private:
            std::stack<GLTarget> targets_stack;
    };
}

#endif // _BTK_GL_HPP_
