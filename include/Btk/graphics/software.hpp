#if !defined(_BTK_GL_SOFTWARE_HPP_)
#define _BTK_GL_SOFTWARE_HPP_
#include "../render.hpp"
#include "../pixels.hpp"
#include <stack>
struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;
namespace Btk{
    /**
     * @brief Software renderer by using nanort
     * 
     */
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
            void set_target(SDL_Surface *surf);
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
            void active_env();
            // void sw_flush();
            void sw_resize(int new_w,int new_h);

            // void hw_flush();
            void hw_resize(int new_w,int new_h);

            //Target
            struct Target{
                void *pixels;
                int   w;
                int   h;
            };
            using Stack = std::stack<Target>;

            Stack         target_stack;
            bool          target_attached = false;
            //HW Bilt
            SDL_Renderer *hw_render = nullptr;
            SDL_Texture  *hw_framebuffer = nullptr;//< Renderer buf for sw_ctxt
            bool          hw_attached = false;//< The sw_ctxt attached the buffer?
            bool          hw_owned = false;
            bool          hw_bilt = false;
            //SW Bilt
            SDL_Surface  *sw_framebuffer = nullptr;//< Renderer buf for sw_ctxt
            bool          sw_owned = false;
            //Window
            SDL_Surface *window_framebuffer = nullptr;//< Screen
            SDL_Window  *window = nullptr;
            //Context
            NVGcontext  *sw_context = nullptr;
    };
}

#endif // _BTK_GL_SOFTWARE_HPP_
