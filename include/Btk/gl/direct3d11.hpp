#if !defined(_BTK_DIRECT3D11_HPP_)
#define _BTK_DIRECT3D11_HPP_
#include "../platform/win32.hpp"
#include <d3d11.h>
namespace Btk{
    /**
     * @brief Direct3D 11 Device
     * 
     */
    struct BTKAPI DxDevice:public RendererDevice{
        DxDevice(HWND window);
        DxDevice(const DxDevice &) = delete;
        ~DxDevice();

        template<class T>
        using ComInstance = Win32::ComInstance<T>;
        
        HWND window;
        //Device,finnal released
        ComInstance<ID3D11Device> device;

        ComInstance<ID3D11RenderTargetView> render_view;
        ComInstance<ID3D11DepthStencilView> stencil_view;
        ComInstance<ID3D11Texture2D>        stencil;
        //Should we add depth view
        //ComInstance<ID3D11DepthStencilView> depth_view;
        ComInstance<ID3D11DeviceContext> context;
        ComInstance<IDXGISwapChain> swapchain;
        ComInstance<ID3D11Texture2D> buffer;

        //Renderer target for user
        struct DxTarget{
            ComInstance<ID3D11RenderTargetView> render_view;
            ComInstance<ID3D11DepthStencilView> stencil_view;
            ComInstance<ID3D11Texture2D>        stencil;
            ComInstance<ID3D11Texture2D>        texture;//Current texture
        };

        //Current framebuffer size for HWND
        UINT buf_w;
        UINT buf_h;
        //Buffer
        void clear_buffer(Color c) override;
        void swap_buffer() override;
        bool output_size(
            Size *p_logical_size,
            Size *p_physical_size
        );
        //Context
        Context create_context() override;
        void    destroy_context(Context) override;
        //Viewport
        void set_viewport(const Rect *r) override;
        //Target
        void set_target(Context ctxt,TextureID id);
        /**
         * @brief Reset
         * 
         * @param ctxt 
         */
        void reset_target(Context ctxt);
        //Texture
        TextureID clone_texture(Context ctxt,TextureID);
        bool      query_texture(Context ctxt,
                                TextureID id,
                                Size *p_size,
                                void *p_handle,
                                TextureFlags *p_flags);
        //Internal method
        void resize_buffer(UINT new_w,UINT new_h);
    };

}


#endif // _BTK_DIRECT3D11_HPP_
