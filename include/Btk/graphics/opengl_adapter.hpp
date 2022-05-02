#if !defined(_BTK_OPENGL_ADAPTER_HPP_)
#define _BTK_OPENGL_ADAPTER_HPP_
#include "../defs.hpp"
namespace Btk{
    struct GLVersion{
        int  major;
        int  minor;
        bool es;
    };
    /**
     * @brief Adapter for GLDevice
     * 
     */
    class GLAdapter{
        public:
            virtual BTKFAST ~GLAdapter(){};
            virtual BTKFAST void   initialize(void *win_handle) = 0;
            //Env
            virtual BTKFAST void  *get_proc(const char *name) = 0;
            virtual BTKFAST void   get_drawable(int *w,int *h) = 0;
            virtual BTKFAST void   get_window_size(int *w,int *h) = 0;
            virtual BTKFAST bool   has_extension(const char *extname) = 0;
            virtual BTKFAST void   swap_buffer() = 0;

            virtual BTKFAST void   begin_context() = 0;
            virtual BTKFAST void   end_context() = 0;
            virtual BTKFAST void   make_current() = 0;

            BTKAPI
            GLVersion get_version();
    };
}

#endif // _BTK_OPENGL_ADAPTER_HPP_
