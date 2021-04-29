#include "../build.hpp"

#include <d3d11.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_loadso.h>

#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/Btk.hpp>

//Win32 utils
#include "../platform/win32/internal.hpp"

extern "C"{
    #define NANOVG_D3D11_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_d3d11.h"
}
//TODO Finish it
#define DX_DEVICE() device->device
#define DX_CONTEXT() device->context
#define DX_SWAPCHAIN() device->swapchain
#define DX_RENDERVIEW() device->render_view


namespace Btk{
    
    struct BTKHIDDEN RendererDevice{
        RendererDevice(SDL_Window *win);
        ~RendererDevice();

        template<class T>
        using ComInstance = Win32::ComInstance<T>;
        
        SDL_Window *win;
        
        ComInstance<ID3D11RenderTargetView> render_view;
        //Should we add depth view
        //ComInstance<ID3D11DepthStencilView> depth_view;
        ComInstance<IDXGISwapChain> swapchain;
        ComInstance<ID3D11DeviceContext> context;
        ComInstance<ID3D11Device> device;
        //Current buffer size
        UINT buf_w;
        UINT buf_h;
    };
    Renderer::Renderer(SDL_Window *win){
        //Create d3d context
        device = nullptr;
        device = new RendererDevice(win);
        nvg_ctxt = nvgCreateD3D11(device->device,NVG_STENCIL_STROKES | NVG_ANTIALIAS);
        if(nvg_ctxt == nullptr){
            //HandleErrr
            //throwRendererError();
        }
        window = win;
    }
    Renderer::~Renderer(){
        destroy();
    }
    void Renderer::destroy(){
        if(nvg_ctxt != nullptr){
            nvgDeleteD3D11(nvg_ctxt);
            //Release the d3d device
            delete device;
            nvg_ctxt = nvg_ctxt;
            device = nullptr;
        }
    }
    RendererBackend Renderer::backend() const{
        return RendererBackend::Dx11;
    }

    void Renderer::clear(Color c){
        const FLOAT color[] = {
            1.0f / 255 * c.r, 
            1.0f / 255 * c.g, 
            1.0f / 255 * c.b, 
            1.0f / 255 * c.a 
        };
        DX_CONTEXT()->ClearRenderTargetView(DX_RENDERVIEW(),color);
    }
    void Renderer::swap_buffer(){
        DX_SWAPCHAIN()->Present(0,0);
    }
    void Renderer::begin(){
        int w,h;
        SDL_GetWindowSize(window,&w,&h);

        if(w != device->buf_w or h != device->buf_h){
            //Resize the buf
            DX_SWAPCHAIN()->ResizeBuffers(1,w,h,DXGI_FORMAT_UNKNOWN,0);
            device->buf_h = h;
            device->buf_w = w;
            //Reset the viewport
            D3D11_VIEWPORT viewport;
            viewport.Height = h;
            viewport.Width = w;
            viewport.MaxDepth = 1.f;
            viewport.MinDepth = 0.f;
            viewport.TopLeftX = 0.f;
            viewport.TopLeftY = 0.f;

            DX_CONTEXT()->RSSetViewports(1,&viewport);
        }
        nvgBeginFrame(nvg_ctxt,w,h,1);
    }
}
namespace Btk{
    using D3D11InitFn = decltype(D3D11CreateDevice);
    static void *d3d11_lib = nullptr;
    static D3D11InitFn *d3d11_create;
    static void load_d3d11(){
        if(d3d11_lib != nullptr){
            d3d11_lib = SDL_LoadObject("D3D11.dll");
            if(d3d11_lib == nullptr){
                throwRendererError(SDL_GetError());
            }
            //Get pointer
            d3d11_create = reinterpret_cast<D3D11InitFn*>(
                SDL_LoadFunction(d3d11_lib,"D3D11CreateDevice")
            );
            AtExit(SDL_UnloadObject,d3d11_lib);
        }
    }
    RendererDevice::RendererDevice(SDL_Window *win){
        SDL_SysWMinfo info;
        SDL_GetVersion(&info.version);
        SDL_GetWindowWMInfo(win,&info);

        BTK_ASSERT(info.subsystem == SDL_SYSWM_WINDOWS);

        HWND handle = info.info.win.window;
        load_d3d11();
        //Init d3d11
        const D3D_FEATURE_LEVEL level[] = {
            D3D_FEATURE_LEVEL_11_0
        };
        //Current level
        D3D_FEATURE_LEVEL cur;
        HRESULT ret = d3d11_create(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            0,
            level,
            SDL_arraysize(level),
            D3D11_SDK_VERSION,
            &device,
            &cur,
            &context
        );
        if(FAILED(ret)){
            throwRendererError("Could not create d3d11 device");
        }
        this->win = win;
        //Get factory
        ComInstance<IDXGIDevice> dxgi_dev;
        ComInstance<IDXGIAdapter> dxgi_apt;
        ComInstance<IDXGIFactory> dxgi_fct;
        
        ret = device->QueryInterface(&dxgi_dev);
        ret = dxgi_dev->GetParent(__uuidof(IDXGIAdapter),reinterpret_cast<void**>(&dxgi_apt));
        ret = dxgi_apt->GetParent(__uuidof(IDXGIFactory),reinterpret_cast<void**>(&dxgi_fct));
        //Do check...
        //Create SwapChain
        SDL_DisplayMode display;
        SDL_GetWindowDisplayMode(win,&display);
        int w;
        int h;
        SDL_GetWindowSize(win,&w,&h);

        buf_w = w;
        buf_h = h;

        DXGI_SWAP_CHAIN_DESC desc;
        SDL_zero(desc);
        //Buffer
        desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferDesc.Width = w;
        desc.BufferDesc.Height = h;
        desc.BufferDesc.RefreshRate.Denominator = 1;
        desc.BufferDesc.RefreshRate.Numerator = display.refresh_rate;
        desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        //Sample
        desc.SampleDesc.Count = 4;
        desc.SampleDesc.Quality = 0;

        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 1;
        desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        if(SDL_GetWindowFlags(win) & SDL_WINDOW_FULLSCREEN == SDL_WINDOW_FULLSCREEN){
            //Full screen
            desc.Windowed = TRUE;
        }
        else{
            desc.Windowed = FALSE;
        }
        desc.OutputWindow = handle;
        ret = dxgi_fct->CreateSwapChain(dxgi_dev,&desc,&swapchain);
        //SwapChain end
        //Create view
        ComInstance<ID3D11Buffer> buffer;
        ret = swapchain->GetBuffer(0,__uuidof(ID3D11Buffer),reinterpret_cast<void**>(&buffer));
        ret = device->CreateRenderTargetView(buffer,nullptr,&render_view);
        //Bind it
        context->OMSetRenderTargets(1,&render_view,nullptr);
        //ViewPort
        D3D11_VIEWPORT viewport;
        viewport.Height = h;
        viewport.Width = w;
        viewport.MaxDepth = 1.f;
        viewport.MinDepth = 0.f;
        viewport.TopLeftX = 0.f;
        viewport.TopLeftY = 0.f;

        context->RSSetViewports(1,&viewport);
    }
    RendererDevice::~RendererDevice(){

    }
}