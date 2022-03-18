#include "../build.hpp"

#include <d3d11.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_loadso.h>

#include <Btk/platform/win32.hpp>
#include <Btk/gl/direct3d11.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/canvas.hpp>
#include <Btk/Btk.hpp>


extern "C"{
    #define NANOVG_D3D11_IMPLEMENTATION
    #include "../libs/nanovg.h"
    #include "../libs/nanovg_d3d11.h"
}
#define BTK_THROW_HRESULT(HR) \
    check_hr(HR);
#if !defined(NDEBUG) && !BTK_MINGW
    #define BTK_D3D11_SET_DEBUG_NAME(OBJ,NAME) \
        OBJ->SetPrivateData(WKPDID_D3DDebugObjectName,strlen(NAME)-1,NAME)
#else
    #define BTK_D3D11_SET_DEBUG_NAME(OBJ,NAME)
#endif

//Utility for D3D11
namespace {
    using D3D11InitFn = decltype(D3D11CreateDeviceAndSwapChain);
    static void *d3d11_lib = nullptr;
    static HHOOK d3d11_hook = nullptr;
    static D3D11InitFn *d3d11_create;
    static Btk::Constructable<Btk::Win32::HWNDMap<Btk::DxDevice*>> hdevs_map;
    //Begin
    static Btk::Size win32_win_size(HWND win){
        RECT rect;
        GetClientRect(win,&rect);
        return {
            rect.right - rect.left,
            rect.bottom - rect.top
        };
    }
    //Map HWND to DxDevice
    static LRESULT CALLBACK dx_msghook(int code, WPARAM wParam, LPARAM lParam){
        // BTK_ASSERT(code == WH_CALLWNDPROC);
        CWPSTRUCT *window = reinterpret_cast<CWPSTRUCT*>(lParam);
        Btk::DxDevice *device = nullptr;
        switch(window->message){
            //We only query the map when the WM_SIZING or WM_SIZE
            case WM_SIZING:{
                device = hdevs_map->find(window->hwnd);
                if(device == nullptr){
                    //Is not the hwnd we managed
                    break;
                }
                //TODO Improve algo here
                BTK_LOGINFO("[DxDevice]WM SIZING for device %p",device);
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
                BTK_LOGINFO("[DxDevice]WM SIZE for device %p",device);
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
            if(FAILED(hr)){
                Btk::throwWin32Error(DWORD(hr));
            }
            d3d11_lib = SDL_LoadObject("D3D11.dll");
            if(d3d11_lib == nullptr){
                Btk::throwRendererError(SDL_GetError());
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
            Btk::AtExit(quit_d3d11);
        }
    }
    static int win32_refresh_rate(HWND hw){
        HDC dc = GetDC(hw);
        int rate = GetDeviceCaps(dc,VREFRESH);
        ReleaseDC(hw,dc);
        BTK_LOGINFO("[GDI]Get VRefresh rate %d at %p",rate,hw);
        return rate;
    }
    static auto dxdev_findtexture(NVGcontext *ctxt,Btk::TextureID id){
        auto d3d = static_cast<D3DNVGcontext*>(nvgInternalParams(ctxt)->userPtr);
        auto texture = D3Dnvg__findTexture(d3d,id);
        if(texture == nullptr){
            Btk::throwRendererError("Invaid texture");
        }
        return texture;
    }
    static auto dxdev_alloctexture(NVGcontext *ctxt){
        auto d3d = static_cast<D3DNVGcontext*>(nvgInternalParams(ctxt)->userPtr);
        auto texture = D3Dnvg__allocTexture(d3d);
        return texture;
    }
}
namespace Btk{
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
        #if !defined(NDEBUG) && !BTK_MINGW
        dev_flag = D3D11_CREATE_DEVICE_DEBUG;
        #endif

        const D3D_FEATURE_LEVEL req_levels [] = {
            D3D_FEATURE_LEVEL_11_0,
        };
        auto dev_types = {
            D3D_DRIVER_TYPE_HARDWARE,//< First try hardware
            D3D_DRIVER_TYPE_WARP,//< If hardware not found, try WARP
            D3D_DRIVER_TYPE_SOFTWARE//< If WARP not found, try software
        };
        for(auto type:dev_types){
            hr = d3d11_create(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                dev_flag,
                req_levels,
                SDL_arraysize(req_levels),
                D3D11_SDK_VERSION,
                &desc,
                &swapchain,
                &device,
                nullptr,
                &context
            );
            if(SUCCEEDED(hr)){
                break;
            }
        }
        BTK_THROW_HRESULT(hr);
        //Get Buffer
        resize_buffer(w,h);
        //Begin hook
        //Register self
        hdevs_map->insert(window,this);
    }
    DxDevice::~DxDevice(){
        hdevs_map->erase(window);
    }

    auto DxDevice::create_context() -> Context{
        return nvgCreateD3D11(device,NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    }
    void DxDevice::destroy_context(Context ctxt){
        nvgDeleteD3D11(ctxt);
    }
    void DxDevice::clear_buffer(Color c){
        const FLOAT color[] = {
            1.0f / 255 * c.r, 
            1.0f / 255 * c.g, 
            1.0f / 255 * c.b, 
            1.0f / 255 * c.a 
        };
        ID3D11RenderTargetView *rview;
        ID3D11DepthStencilView *sview;
        if(not targets.empty()){
            //Using render target
            rview = targets.top().render_view;
            sview = targets.top().stencil_view;
        }
        else{
            //using swap chain
            rview = render_view;
            sview = stencil_view;
        }
        context->ClearRenderTargetView(rview,color);
        context->ClearDepthStencilView(sview,D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,0.0f,0);
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
    void DxDevice::resize_buffer(UINT new_w,UINT new_h){
        BTK_LOGINFO("Resize DxBuffer to %u %u",new_w,new_h);

        //Check 
        if(not has_internal_target_binded){
            return;
        }
        if(new_h == 0 or new_h == 0){
            return;
        }
        //Process with ssaa ratio
        new_w = UINT(float(new_w) * ssaa_ratio);
        new_h = UINT(float(new_h) * ssaa_ratio);
        if(new_h == buf_h and new_w == buf_w){
            //Is same size
            return;
        }

        buf_w = new_w;
        buf_h = new_h;
        //Here are some code from d3d11 nvg demo
        //Release prev view
        HRESULT hr;
        //Reset binding
        unbind();

        stencil_view.release();
        render_view.release();
        stencil.release();
        buffer.release();

        //SwapChain
        hr = swapchain->ResizeBuffers(1,new_w,new_h,DXGI_FORMAT_R8G8B8A8_UNORM,0);
        BTK_THROW_HRESULT(hr);
        //Get buffer
        hr = swapchain->GetBuffer(0,__uuidof(ID3D11Texture2D),reinterpret_cast<void**>(&buffer));
        BTK_THROW_HRESULT(hr);
        //Make view
        render_view = create_render_target_view(buffer);
        //Make stencil buffer
        stencil = create_stencil_buffer(new_w,new_h);
        //Make stencil view
        stencil_view = create_stencil_view(stencil);

        //set render target now
        bind();

        //Mark name
        BTK_D3D11_SET_DEBUG_NAME(buffer,"DxDevice swapchain buffer");
        BTK_D3D11_SET_DEBUG_NAME(render_view,"DxDevice swapchain render view");
        BTK_D3D11_SET_DEBUG_NAME(stencil,"DxDevice swapchain stencil");
        BTK_D3D11_SET_DEBUG_NAME(stencil_view,"DxDevice swapchain stencil view");

    }
    auto DxDevice::create_stencil_buffer(UINT w,UINT h) -> ComInstance<ID3D11Texture2D> {
        //Make stencil buffer
        ComInstance<ID3D11Texture2D> stencil;
        D3D11_TEXTURE2D_DESC stencil_desc;
        HRESULT hr;
        
        stencil_desc.ArraySize = 1;
        stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        stencil_desc.CPUAccessFlags = 0;
        stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        stencil_desc.Height = h;
        stencil_desc.Width = w;
        stencil_desc.MipLevels = 1;
        stencil_desc.MiscFlags = 0;
        stencil_desc.SampleDesc.Count = 1;
        stencil_desc.SampleDesc.Quality = 0;
        stencil_desc.Usage = D3D11_USAGE_DEFAULT;

        hr = device->CreateTexture2D(&stencil_desc,nullptr,&stencil);
        BTK_THROW_HRESULT(hr);

        return stencil;
    }
    auto DxDevice::create_stencil_view(ID3D11Texture2D *texture) -> ComInstance<ID3D11DepthStencilView>{
        //Make stencil view
        ComInstance<ID3D11DepthStencilView> stencil_view;
        D3D11_DEPTH_STENCIL_VIEW_DESC stencil_vdesc;
        D3D11_TEXTURE2D_DESC tex_desc;
        HRESULT hr;

        texture->GetDesc(&tex_desc);

        stencil_vdesc.Format = tex_desc.Format;
        stencil_vdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        stencil_vdesc.Flags = 0;
        stencil_vdesc.Texture2D.MipSlice = 0;

        hr = device->CreateDepthStencilView(
            texture,
            &stencil_vdesc,
            &stencil_view
        );
        BTK_THROW_HRESULT(hr);
        return stencil_view;
    }
    auto DxDevice::create_render_target_view(ID3D11Texture2D *texture) -> ComInstance<ID3D11RenderTargetView>{
        //Make view
        ComInstance<ID3D11RenderTargetView> render_view;
        D3D11_RENDER_TARGET_VIEW_DESC view_desc;
        D3D11_TEXTURE2D_DESC tex_desc;
        HRESULT hr;

        //Mainly for pixel format
        texture->GetDesc(&tex_desc);

        view_desc.Format = tex_desc.Format;
        view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        view_desc.Texture2D.MipSlice = 0;

        hr = device->CreateRenderTargetView(
            texture,
            &view_desc,
            &render_view
        );
        BTK_THROW_HRESULT(hr);

        return render_view;
    }
    //TODO:Need we set viewport?
    void DxDevice::unbind(){
        ID3D11RenderTargetView *empty_view[1] = {nullptr};
        //Reset binding
        context->OMSetRenderTargets(1,empty_view,nullptr);
        has_internal_target_binded = false;
    }
    void DxDevice::bind(){
        if(targets.empty()){
            context->OMSetRenderTargets(1,&render_view,stencil_view);
        }
        else{
            auto &top = targets.top();
            context->OMSetRenderTargets(1,&top.render_view,top.stencil_view);
        }
        has_internal_target_binded = true;
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
        if(desc.MipLevels == 0){
            context->GenerateMips(new_texture->resourceView);
        }
        return new_texture->id;
    }
    void DxDevice::set_target(Context ctxt,TextureID id){
        if(not has_internal_target_binded){
            throwRendererError("No internal target binded");
        }
        auto texture = dxdev_findtexture(ctxt,id);
        //End current frame
        end_frame(ctxt);
        //Unbind
        ID3D11RenderTargetView *empty_view[1] = {nullptr};
        context->OMSetRenderTargets(1,empty_view,0);

        //Set up a new target
        DxTarget target;
        target.stencil = create_stencil_buffer(texture->height,texture->width);
        target.render_view = create_render_target_view(texture->tex);
        target.stencil_view = create_stencil_view(target.stencil);
        target.texture = texture->tex;
        targets.emplace(target);

        //Mark texture name for debug
        BTK_D3D11_SET_DEBUG_NAME(target.stencil,u8format("stencil for tex:%d",id).c_str());
        BTK_D3D11_SET_DEBUG_NAME(target.render_view,u8format("render view for tex:%d",id).c_str());
        BTK_D3D11_SET_DEBUG_NAME(target.stencil_view,u8format("stencil view for tex:%d",id).c_str());
        BTK_D3D11_SET_DEBUG_NAME(target.texture,u8format("texture for tex:%d",id).c_str());

        //Add ref count(Because dx target will be released when pop)
        texture->tex->AddRef();
        //Bind
        context->OMSetRenderTargets(1,&target.render_view,target.stencil_view);
        //Set viewport
        D3D11_VIEWPORT vp;
        vp.Width = texture->width;
        vp.Height = texture->height;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        context->RSSetViewports(1,&vp);
        //Begin a new frame
        begin_frame_ex(ctxt,texture->width,texture->height,1);
        //Save current state
        nvgSave(ctxt);
    }
    void DxDevice::reset_target(Context ctxt){
        if(not has_internal_target_binded){
            throwRendererError("No internal target binded");
        }
        if(targets.empty()){
            return;
        }
        //FIXME:D3D11 Warning The Pixel Shader unit expects a Sampler at slot 0.
        //End current frame
        end_frame(ctxt);
        nvgRestore(ctxt);
        //Unbind
        ID3D11RenderTargetView *empty_view[1] = {nullptr};
        context->OMSetRenderTargets(1,empty_view,nullptr);
        //Pop target
        targets.pop();
        //Begin a new frame according to the top target
        if(targets.empty()){
            //To set up internal target
            auto [win_w,win_h] = win32_win_size(window);
            context->OMSetRenderTargets(1,&render_view,stencil_view);
            set_viewport(nullptr);
            begin_frame_ex(
                ctxt,
                win_w,
                win_h,
                float(buf_w) / float(win_w)
            );
        }
        else{
            auto &top = targets.top();
            //Get texture width and height
            D3D11_TEXTURE2D_DESC tex_desc;
            top.texture->GetDesc(&tex_desc);
            //Bind
            context->OMSetRenderTargets(1,&top.render_view,top.stencil_view);
            //Viewport
            D3D11_VIEWPORT vp;
            vp.Width = tex_desc.Width;
            vp.Height = tex_desc.Height;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.TopLeftX = 0;
            vp.TopLeftY = 0;
            context->RSSetViewports(1,&vp);
            //Begin a new frame
            begin_frame_ex(ctxt,tex_desc.Width,tex_desc.Height,1);
        }
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
        //Process viewport by ssaa ratio
        if(ssaa_ratio > 1){
            viewport.Height *= ssaa_ratio;
            viewport.Width *= ssaa_ratio;
            viewport.TopLeftX *= ssaa_ratio;
            viewport.TopLeftY *= ssaa_ratio;
        }

        context->RSSetViewports(1,&viewport);
    }
    void *DxDevice::lock_texture(
        Context ctxt,
        TextureID id,
        const Rect *r,
        LockFlag flag
    ){
        BTK_FIXME("DxDevice::lock_texture temporarily unsupport Read flag");
        if((flag & LockFlag::Write) == LockFlag::Write){
            // throwRendererError("LockFlag::WRITE is not supported");
            return nullptr;
        }

        auto texture = dxdev_findtexture(ctxt,id);
        if(texture == nullptr){
            return nullptr;
        }
        ComInstance<ID3D11Texture2D> dtex;
        //Copy texture to dtex
        D3D11_TEXTURE2D_DESC desc;

        texture->tex->GetDesc(&desc);
        if(desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM){
            // throwRendererError("Unsupported texture format");
            return nullptr;
        }

        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

        //Modify desc by r
        if(r != nullptr){
            desc.Width = r->w;
            desc.Height = r->h;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.BindFlags = 0;
            desc.MiscFlags = 0;
        }
        else{
            desc.Width = texture->width;
            desc.Height = texture->height;
            desc.MipLevels = 1;
            desc.ArraySize = 1;
            desc.BindFlags = 0;
            desc.MiscFlags = 0;
        }

        //Create a staging texture
        if(FAILED(device->CreateTexture2D(&desc,nullptr,&dtex))){
            return nullptr;
        }
        BTK_D3D11_SET_DEBUG_NAME(dtex,"TmpTexture for lock");
        //Copy texture to staging texture by rect
        if(r == nullptr){
            context->CopyResource(dtex,texture->tex);
        }
        else{
            D3D11_BOX box;
            box.left = r->x;
            box.top = r->y;
            box.right = r->x + r->w;
            box.bottom = r->y + r->h;
            box.front = 0;
            box.back = 1;
            context->CopySubresourceRegion(
                dtex,
                0,
                0,
                0,
                0,
                texture->tex,
                0,
                &box
            );
        }
        //Begin map
        D3D11_MAPPED_SUBRESOURCE map;
        if(FAILED(context->Map(dtex,0,D3D11_MAP_READ,0,&map))){
            return nullptr;
        }
        //Create a memory buffer
        auto buf = new uint32_t[desc.Width * desc.Height];
        //Copy data by pitch
        SDL_ConvertPixels(
            desc.Width,
            desc.Height,
            SDL_PIXELFORMAT_RGBA32,
            map.pData,
            map.RowPitch,
            SDL_PIXELFORMAT_RGBA32,
            buf,
            desc.Width * sizeof(uint32_t)
        );
        //End map
        context->Unmap(dtex,0);
        //Return
        return buf;
    }
    void  DxDevice::unlock_texture(
        Context ctxt,
        TextureID id,
        void *pixels
    ){
        //Free memory
        delete[] static_cast<uint32_t*>(pixels);
    }
    void DxDevice::check_hr(HRESULT hr){
        if(FAILED(hr)){
            set_error(Win32::StrMessageA(DWORD(hr)));
            throwRendererError(get_error());   
        }
    }
    RendererDevice *CreateD3D11Device(HWND h){
        return new DxDevice(h);
    }
}

namespace Btk{
    //DxCanvas
    void DxCanvas::set_parent(Widget *parent){
        if(parent == nullptr and is_inited){
            dx_release();
            is_inited = false;
        }
        Widget::set_parent(parent);
        if(renderer() == nullptr){
            return;
        }
        prepare_device();
    }
    void DxCanvas::prepare_device(){
        auto dev = renderer()->device();
        if(dev->backend() != RendererBackend::Dx11){
            throwRendererError("DxCanvas::prepare_device: backend is not D3D11");
        }
        auto dxdev = static_cast<DxDevice*>(dev);
        dx_init();
        is_inited = true;
    }
    void DxCanvas::draw(Renderer &r,Uint32 t){
        auto dev = r.device();
        auto dxdev = static_cast<DxDevice*>(dev);
        if(not is_inited){
            prepare_device();
        }
        auto [win_w,win_h] = dxdev->logical_size();
        auto [phy_w,phy_h] = dxdev->physical_size();

        float w_ratio = float(phy_w) / float(win_w);
        float h_ratio = float(phy_h) / float(win_h);

        dxdev->end_frame(r.context());
        dxdev->unbind();

        //Calc the viewport of by position and size
        D3D11_VIEWPORT viewport;
        viewport.TopLeftX = x() * w_ratio;
        viewport.TopLeftY = y() * h_ratio;
        viewport.Width = w() * w_ratio;
        viewport.Height = h() * h_ratio;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;

        dxdev->context->RSSetViewports(1,&viewport);

        //Draw
        dx_draw(t);

        dxdev->bind();
        //Reset viewport
        dxdev->set_viewport(nullptr);
        dxdev->begin_frame_ex(
            r.context(),
            win_w,win_h,
            w_ratio
        );
        return;
    }
    DxDevice *DxCanvas::dx_device() const{
        return dynamic_cast<DxDevice*>(renderer()->device());
    }
}