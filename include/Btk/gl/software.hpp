#if !defined(_BTK_GL_SOFTWARE_HPP_)
#define _BTK_GL_SOFTWARE_HPP_
#include "../render.hpp"
#include "../pixels.hpp"
namespace Btk{
    class SWDevice:public RendererDevice{
        public:
            SWDevice(SDL_Window *win);
            SWDevice(SDL_Surface *surf,bool owned = false);
            /**
             * @brief Construct a new SWDevice object
             * 
             * @param pixbuf 
             */
            SWDevice(PixBufRef pixbuf):
                SWDevice(pixbuf.get(),false){};
            ~SWDevice();
            //Ctxt
            Context create_context() override;
            void    destroy_context(Context ctxt) override;
            //viewport
            void    set_viewport(const Rect *r) override;
            //Target
            void set_target(Context ctxt,TextureID id) override;
            void reset_target(Context ctxt) override;
            //Texture
            TextureID clone_texture(Context ctxt,TextureID id) override;
            bool      query_texture(Context ctxt,
                                    TextureID id,
                                    Size *p_size,
                                    void *p_handle,
                                    TextureFlags *p_flags) override;
            //Lock etc..
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

            //Frame
            void begin_frame(Context ctxt,
                             float w,
                             float h,
                             float pixel_ratio) override;
            void end_frame(Context ctxt);
            //Buffer
            bool output_size(
                Size *p_logical_size,
                Size *p_physical_size
            ) override;

            void clear_buffer(Color c) override;
            void swap_buffer() override;
        private:
            void update_status();

            NVGcontext *current_ctxt = nullptr;
            SDL_PixelFormat *cvt_fmt = nullptr;
            SDL_Surface *tag_surf = nullptr;
            SDL_Window  *tag_win = nullptr;
            Uint32       tag_fmt = 0;
            bool surf_owned = false;
            bool frame_begined = false;

    };
}

#endif // _BTK_GL_SOFTWARE_HPP_
