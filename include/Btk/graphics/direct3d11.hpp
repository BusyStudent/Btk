#if !defined(_BTK_DIRECT3D11_HPP_)
#define _BTK_DIRECT3D11_HPP_
#include "../platform/win32.hpp"
#include "../render.hpp"
#include <d3d11.h>
#include <stack>
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

        std::stack<DxTarget> targets;
        bool has_internal_target_binded = true;

        //Current framebuffer size for HWND
        UINT buf_w;
        UINT buf_h;

        float ssaa_ratio = 1.0f;

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
        void set_target(Context ctxt,TextureID id) override;
        /**
         * @brief Reset
         * 
         * @param ctxt 
         */
        void reset_target(Context ctxt) override;
        //Texture
        TextureID clone_texture(Context ctxt,TextureID) override;
        bool      query_texture(Context ctxt,
                                TextureID id,
                                Size *p_size,
                                void *p_handle,
                                TextureFlags *p_flags) override;
        //Lock / Unlock
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
        
        //Internal method
        void resize_buffer(UINT new_w,UINT new_h);
        /**
         * @brief Create a stencil buffer object(DXGI_FORMAT_D24_UNORM_S8_UINT)
         * 
         * @param w 
         * @param h 
         * @return ComInstance<ID3D11Texture2D> 
         */
        auto create_stencil_buffer(UINT w,UINT h) -> ComInstance<ID3D11Texture2D>;
        /**
         * @brief Create a stencil view object
         * 
         * @return ComInstance<ID3D11DepthStencilView> 
         */
        auto create_stencil_view(ID3D11Texture2D *) -> ComInstance<ID3D11DepthStencilView>;
        /**
         * @brief Create a render target view object
         * 
         * @return ComInstance<ID3D11RenderTargetView> 
         */
        auto create_render_target_view(ID3D11Texture2D *) -> ComInstance<ID3D11RenderTargetView>;

        /**
         * @brief Unbind current render target
         * 
         */
        void unbind();
        /**
         * @brief Bind to default render target
         * 
         */
        void bind();

        void check_hr(HRESULT hr);
    };

}


#endif // _BTK_DIRECT3D11_HPP_
