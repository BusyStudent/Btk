#include "../build.hpp"

#include <d3d11.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_loadso.h>

#include <Btk/platform/win32.hpp>
#include <Btk/gl/direct3d11.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/Btk.hpp>


extern "C"{
    #define NANOVG_D3D11_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_d3d11.h"
}
#define BTK_THROW_HRESULT(HR) \
    if(FAILED(hr)){\
        Btk::throwRendererError(Btk::Win32::StrMessageA(DWORD(hr)));\
    }

namespace Btk{
    static inline Size win32_win_size(HWND win){
        RECT rect;
        GetClientRect(win,&rect);
        return {
            rect.right - rect.left,
            rect.bottom - rect.top
        };
    }
    DxDevice::Context DxDevice::create_context(){
        return nvgCreateD3D11(device,NVG_ANTIALIAS);
    }
    void DxDevice::destroy_context(Context ctxt){
        nvgDeleteD3D11(ctxt);
    }
    // void DxDevice::set_viewport(const Rect *r){
    //     D3D11_VIEWPORT viewport;
    //     if(r == nullptr){
    //         //Reset
    //     }
    //     else{
    //         viewport.Width  = r->w;
    //         viewport.Height = r->h;
    //     }
    //     context->RSSetViewports(1,&viewport);
    // }
    // Renderer::Renderer(SDL_Window *win){
    //     //Create d3d context
    //     device = nullptr;
    //     device = new RendererDevice(win);
    //     nvg_ctxt = nvgCreateD3D11(device->device,NVG_STENCIL_STROKES | NVG_ANTIALIAS);
    //     if(nvg_ctxt == nullptr){
    //         //HandleErrr
    //         //throwRendererError();
    //     }
    //     window = win;
    // }
    // Renderer::~Renderer(){
    //     destroy();
    // }
    // void Renderer::destroy(){
    //     if(nvg_ctxt != nullptr){
    //         nvgDeleteD3D11(nvg_ctxt);
    //         //Release the d3d device
    //         delete device;
    //         nvg_ctxt = nvg_ctxt;
    //         device = nullptr;
    //     }
    // }
    // RendererBackend Renderer::backend() const{
    //     return RendererBackend::Dx11;
    // }
    void DxDevice::clear_buffer(Color c){
        const FLOAT color[] = {
            1.0f / 255 * c.r, 
            1.0f / 255 * c.g, 
            1.0f / 255 * c.b, 
            1.0f / 255 * c.a 
        };
        context->ClearRenderTargetView(render_view,color);
        context->ClearDepthStencilView(stencil_view,D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,0.0f,0);
    }
    void DxDevice::swap_buffer(){
        swapchain->Present(0,0);
    }
    bool DxDevice::output_size( 
            Size *p_logical_size,
            Size *p_physical_size){
        if(p_logical_size != nullptr){
            *p_logical_size = win32_win_size(window);
        }
        if(p_physical_size != nullptr){
            p_physical_size->w = buf_w;
            p_physical_size->h = buf_h;
        }
        return true;
    }
    // void Renderer::begin(){
    //     int w,h;
    //     SDL_GetWindowSize(window,&w,&h);

    //     if(w != device->buf_w or h != device->buf_h){
    //         //Resize the buf
    //         DX_SWAPCHAIN()->ResizeBuffers(1,w,h,DXGI_FORMAT_UNKNOWN,0);
    //         device->buf_h = h;
    //         device->buf_w = w;
    //         //Reset the viewport
    //         D3D11_VIEWPORT viewport;
    //         viewport.Height = h;
    //         viewport.Width = w;
    //         viewport.MaxDepth = 1.f;
    //         viewport.MinDepth = 0.f;
    //         viewport.TopLeftX = 0.f;
    //         viewport.TopLeftY = 0.f;

    //         DX_CONTEXT()->RSSetViewports(1,&viewport);
    //     }
    //     nvgBeginFrame(nvg_ctxt,w,h,1);
    // }
}
namespace Btk{
    using D3D11InitFn = decltype(D3D11CreateDeviceAndSwapChain);
    static void *d3d11_lib = nullptr;
    static HHOOK d3d11_hook = nullptr;
    static D3D11InitFn *d3d11_create;
    static Constructable<Win32::HWNDMap<DxDevice*>> hdevs_map;
    //Map HWND to DxDevice
    static LRESULT CALLBACK dx_msghook(int code, WPARAM wParam, LPARAM lParam){
        // BTK_ASSERT(code == WH_CALLWNDPROC);
        CWPSTRUCT *window = reinterpret_cast<CWPSTRUCT*>(lParam);
        DxDevice *device = nullptr;
        switch(window->message){
            //We only query the map when the WM_SIZING or WM_SIZE
            case WM_SIZING:{
                device = hdevs_map->find(window->hwnd);
                if(device == nullptr){
                    //Is not the hwnd we managed
                    break;
                }
                //TODO Improve algo here
                BTK_LOGINFO("[DxDevice]WM SIZING");
                auto[w,h] = win32_win_size(window->hwnd);
                device->resize_buffer(w,h);
                break;
            }
            case WM_SIZE:{
                device = hdevs_map->find(window->hwnd);
                if(device == nullptr){
                    //Is not the hwnd we managed
                    break;
                }
                BTK_LOGINFO("[DxDevice]WM SIZE");
                device->resize_buffer(LOWORD(window->lParam),HIWORD(window->lParam));
                break;
            }
            default:{
                break;
            }
        }
        return CallNextHookEx(d3d11_hook,code,wParam,lParam);
    }
    static void quit_d3d11(){
        UnhookWindowsHookEx(d3d11_hook);
        hdevs_map.destroy();
        SDL_UnloadObject(d3d11_lib);
        CoUninitialize();
    }
    static void load_d3d11(){
        if(d3d11_lib == nullptr){
            HRESULT hr = CoInitializeEx(0,COINIT_MULTITHREADED);
            BTK_THROW_HRESULT(hr);
            d3d11_lib = SDL_LoadObject("D3D11.dll");
            if(d3d11_lib == nullptr){
                throwRendererError(SDL_GetError());
            }
            //Get pointer
            d3d11_create = reinterpret_cast<D3D11InitFn*>(
                SDL_LoadFunction(d3d11_lib,"D3D11CreateDeviceAndSwapChain")
            );
            hdevs_map.construct();
            //Init hooks
            d3d11_hook = SetWindowsHookEx(
                WH_CALLWNDPROC,
                dx_msghook,
                nullptr,
                GetCurrentThreadId()
            );
            AtExit(quit_d3d11);
        }
    }
//     DxDevice::DxDevice(SDL_Window *win){
//         SDL_SysWMinfo info;
//         SDL_GetVersion(&info.version);
//         SDL_GetWindowWMInfo(win,&info);

//         BTK_ASSERT(info.subsystem == SDL_SYSWM_WINDOWS);

//         HWND handle = info.info.win.window;
//         load_d3d11();
//         //Init d3d11
//         const D3D_FEATURE_LEVEL level[] = {
//             D3D_FEATURE_LEVEL_11_0
//         };
//         //Current level
//         D3D_FEATURE_LEVEL cur;
//         HRESULT ret = d3d11_create(
//             nullptr,
//             D3D_DRIVER_TYPE_HARDWARE,
//             nullptr,
//             0,
//             level,
//             SDL_arraysize(level),
//             D3D11_SDK_VERSION,
//             &device,
//             &cur,
//             &context
//         );
//         if(FAILED(ret)){
//             throwRendererError("Could not create d3d11 device");
//         }
//         this->win = win;
//         //Get factory
//         ComInstance<IDXGIDevice> dxgi_dev;
//         ComInstance<IDXGIAdapter> dxgi_apt;
//         ComInstance<IDXGIFactory> dxgi_fct;
        
//         ret = device->QueryInterface(&dxgi_dev);
//         ret = dxgi_dev->GetParent(__uuidof(IDXGIAdapter),reinterpret_cast<void**>(&dxgi_apt));
//         ret = dxgi_apt->GetParent(__uuidof(IDXGIFactory),reinterpret_cast<void**>(&dxgi_fct));
//         //Do check...
//         //Create SwapChain
//         SDL_DisplayMode display;
//         SDL_GetWindowDisplayMode(win,&display);
//         int w;
//         int h;
//         SDL_GetWindowSize(win,&w,&h);

//         buf_w = w;
//         buf_h = h;

//         DXGI_SWAP_CHAIN_DESC desc;
//         SDL_zero(desc);
//         //Buffer
//         desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//         desc.BufferDesc.Width = w;
//         desc.BufferDesc.Height = h;
//         desc.BufferDesc.RefreshRate.Denominator = 1;
//         desc.BufferDesc.RefreshRate.Numerator = display.refresh_rate;
//         desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//         desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//         //Sample
//         desc.SampleDesc.Count = 4;
//         desc.SampleDesc.Quality = 0;

//         desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//         desc.BufferCount = 1;
//         desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//         if(SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN == SDL_WINDOW_FULLSCREEN){
//             //Full screen
//             desc.Windowed = TRUE;
//         }
//         else{
//             desc.Windowed = FALSE;
//         }
//         desc.OutputWindow = handle;
//         ret = dxgi_fct->CreateSwapChain(dxgi_dev,&desc,&swapchain);
//         //SwapChain end
//         //Create view
//         ComInstance<ID3D11Buffer> buffer;
//         ret = swapchain->GetBuffer(0,__uuidof(ID3D11Buffer),reinterpret_cast<void**>(&buffer));
//         ret = device->CreateRenderTargetView(buffer,nullptr,&render_view);
//         //Bind it
//         context->OMSetRenderTargets(1,&render_view,nullptr);
//         //ViewPort
//         D3D11_VIEWPORT viewport;
//         viewport.Height = h;
//         viewport.Width = w;
//         viewport.MaxDepth = 1.f;
//         viewport.MinDepth = 0.f;
//         viewport.TopLeftX = 0.f;
//         viewport.TopLeftY = 0.f;

//         context->RSSetViewports(1,&viewport);
//     }
    static int win32_refresh_rate(HWND hw){
        HDC dc = GetDC(hw);
        int rate = GetDeviceCaps(dc,VREFRESH);
        ReleaseDC(hw,dc);
        BTK_LOGINFO("[GDI]Get VRefresh rate %d at %p",rate,hw);
        return rate;
    }
    DxDevice::DxDevice(HWND win){
        set_backend(RendererBackend::Dx11);
        load_d3d11();
        window = win;
        HRESULT hr;

        auto[w,h] = win32_win_size(win);

        DXGI_SWAP_CHAIN_DESC desc = {};
        //Buffer
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.Width = w;
        desc.BufferDesc.Height = h;
        //TODO ADD code to get refresh rate
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.RefreshRate.Numerator = win32_refresh_rate(win);

        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        //Sample
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;

        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 1;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        //Output
        desc.OutputWindow = win;
        desc.Windowed = TRUE;
        desc.Flags = 0;

        //TODO Use Flip mode
        //So increase the buffer >= 2
        //Fix resize buffer algo

        UINT dev_flag = 0;
        #ifndef NDEBUG
        dev_flag = D3D11_CREATE_DEVICE_DEBUG;
        #endif

        hr = d3d11_create(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            dev_flag,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &desc,
            &swapchain,
            &device,
            nullptr,
            &context
        );
        BTK_THROW_HRESULT(hr);
        //Get Buffer
        resize_buffer(w,h);
        //Begin hook
        //Register self
        hdevs_map->insert(window,this);
    }
    auto dxdev_findtexture(NVGcontext *ctxt,TextureID id){
        auto d3d = static_cast<D3DNVGcontext*>(nvgInternalParams(ctxt)->userPtr);
        auto texture = D3Dnvg__findTexture(d3d,id);
        if(texture == nullptr){
            throwRendererError("Invaid texture");
        }
        return texture;
    }
    auto dxdev_alloctexture(NVGcontext *ctxt){
        auto d3d = static_cast<D3DNVGcontext*>(nvgInternalParams(ctxt)->userPtr);
        auto texture = D3Dnvg__allocTexture(d3d);
        return texture;
    }
    DxDevice::~DxDevice(){
        hdevs_map->erase(window);
    }
    void DxDevice::resize_buffer(UINT new_w,UINT new_h){
        BTK_LOGINFO("Resize DxBuffer to %u %u",new_w,new_h);

        //Check 
        if(new_h == 0 or new_h == 0){
            return;
        }
        else if(new_h == buf_h and new_w == buf_w){
            //Is same size
            return;
        }

        buf_w = new_w;
        buf_h = new_h;
        //Here are some code from d3d11 nvg demo
        //Release prev view
        HRESULT hr;
        ID3D11RenderTargetView *empty_view[1] = {nullptr};
        //
        context->OMSetRenderTargets(1,empty_view,nullptr);

        stencil_view.release();
        render_view.release();
        stencil.release();
        buffer.release();

        //SwapChain
        hr = swapchain->ResizeBuffers(1,new_w,new_h,DXGI_FORMAT_B8G8R8A8_UNORM,0);
        BTK_THROW_HRESULT(hr);
        //Get buffer
        hr = swapchain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&buffer));
        BTK_THROW_HRESULT(hr);
        //Make view
        D3D11_RENDER_TARGET_VIEW_DESC view_desc;
        view_desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MipSlice = 0;

        hr = device->CreateRenderTargetView(
            buffer,
            &view_desc,
            &render_view
        );
        BTK_THROW_HRESULT(hr);
        //Make stencil buffer
        D3D11_TEXTURE2D_DESC stencil_desc;
        
        stencil_desc.ArraySize = 1;
        stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        stencil_desc.CPUAccessFlags = 0;
        stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        stencil_desc.Height = buf_h;
        stencil_desc.Width = buf_w;
        stencil_desc.MipLevels = 1;
        stencil_desc.MiscFlags = 0;
        stencil_desc.SampleDesc.Count = 1;
        stencil_desc.SampleDesc.Quality = 0;
        stencil_desc.Usage = D3D11_USAGE_DEFAULT;

        hr = device->CreateTexture2D(&stencil_desc,nullptr,&stencil);
        BTK_THROW_HRESULT(hr);

        //Make stencil view
        D3D11_DEPTH_STENCIL_VIEW_DESC stencil_vdesc;

        stencil_vdesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        stencil_vdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        stencil_vdesc.Flags = 0;
        stencil_vdesc.Texture2D.MipSlice = 0;

        hr = device->CreateDepthStencilView(
            stencil,
            &stencil_vdesc,
            &stencil_view
        );
        BTK_THROW_HRESULT(hr);

        //set render target now
        context->OMSetRenderTargets(1,&render_view,stencil_view);
    }
    bool DxDevice::query_texture(Context ctxt,
                                TextureID id,
                                Size *p_size,
                                void *p_handle,
                                TextureFlags *p_flags){
        //find texture
        auto texture = dxdev_findtexture(ctxt,id);
        if(p_size != nullptr){
            p_size->w = texture->width;
            p_size->h = texture->height;
        }
        if(p_handle != nullptr){
            *reinterpret_cast<ID3D11Texture2D**>(p_handle) = texture->tex;
        }
        if(p_flags != nullptr){
            *p_flags = TextureFlags(texture->flags);
        }
        return true;
    }
    TextureID DxDevice::clone_texture(Context ctxt,TextureID id){
        auto texture = dxdev_findtexture(ctxt,id);
        //Get desc and begin copy
        D3D11_SHADER_RESOURCE_VIEW_DESC view_desc;
        D3D11_TEXTURE2D_DESC desc;
        ID3D11Texture2D *dtex;
        HRESULT hr;

        texture->tex->GetDesc(&desc);
        hr = device->CreateTexture2D(&desc,nullptr,&dtex);

        BTK_THROW_HRESULT(hr);
        //Copy resource
        context->CopyResource(dtex,texture->tex);
        //Alloc context texture
        auto new_texture = dxdev_alloctexture(ctxt);
        //TODO 
        new_texture->tex = dtex;
        new_texture->type = texture->type;
        new_texture->flags = texture->flags;
        new_texture->width = texture->width;
        new_texture->height = texture->height;
        new_texture->resourceView = nullptr;
        //Create a view
        view_desc.Format = desc.Format;
        view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MipLevels = UINT(-1);
        view_desc.Texture2D.MostDetailedMip = 0;

        hr = device->CreateShaderResourceView(
            dtex,
            &view_desc,
            &(new_texture->resourceView)
        );
        if(FAILED(hr)){
            nvgDeleteImage(ctxt,new_texture->id);
            BTK_THROW_HRESULT(hr);
        }
        return new_texture->id;
    }
    void DxDevice::set_target(Context ctxt,TextureID id){
        BTK_UNIMPLEMENTED();
    }
    void DxDevice::reset_target(Context ctxt){
        BTK_UNIMPLEMENTED();
    }
    void DxDevice::set_viewport(const Rect *r){
        //TODO 
        D3D11_VIEWPORT viewport;
        
        viewport.MaxDepth = 1.0f;
        viewport.MinDepth = 0.0f;
        if(r == nullptr){
            auto [win_w,win_h] = win32_win_size(window);
            viewport.Height = win_h;
            viewport.Width = win_w;
            viewport.TopLeftX = 0.0f;
            viewport.TopLeftY = 0.0f;
        }
        else{
            viewport.Height = r->h;
            viewport.Width = r->w;
            viewport.TopLeftX = r->x;
            viewport.TopLeftY = r->y;
        }

        context->RSSetViewports(1,&viewport);
    }
    RendererDevice *CreateD3D11Device(HWND h){
        return new DxDevice(h);
    }
}