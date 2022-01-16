#if !defined(_BTK_OPENGL_ADAPTER_HPP_)
#define _BTK_OPENGL_ADAPTER_HPP_
#include "../defs.hpp"
namespace Btk{
    struct GLVersion{
        int  major;
        int  minor;
        bool es;
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
}

#endif // _BTK_OPENGL_ADAPTER_HPP_
