#include "../build.hpp"
#include "adapter.hpp"

#include <Btk/platform/win32.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/impl/codec.hpp>
#include <Btk/Btk.hpp>

#include <Shlwapi.h>
#include <wincodec.h>

namespace{
//TODO::Finish it
struct RwopsIstream:public IStream{
    //IUnknown
    virtual ULONG __stdcall AddRef() override{
        return InterlockedIncrement(&refcount);
    }
    virtual ULONG __stdcall Release() override{
        return InterlockedDecrement(&refcount);
    }
    virtual HRESULT __stdcall QueryInterface(REFIID iid,void**ptr) override{
        //TODO WIC Require it
        if(iid == __uuidof(IUnknown) or
           iid == __uuidof(IStream)  or
           iid == __uuidof(ISequentialStream)){
            *ptr = this;
            AddRef();
            return S_OK;
        }
        return E_NOINTERFACE;
    }
    //ISequentialStream
    virtual HRESULT __stdcall Read(void *buf, ULONG bufsize, ULONG *readed) override{
        size_t ret = SDL_RWread(rwops,buf,1,bufsize);
        *readed = ret;
        if(ret == bufsize){
            return S_OK;
        }
        else{
            return E_FAIL;
        }
    }
    virtual HRESULT __stdcall Write(const void *buf,ULONG bufsize,ULONG *writed) override{
        size_t ret = SDL_RWwrite(rwops,buf,1,bufsize);
        *writed = ret;
        if (ret == bufsize){
            return S_OK;
        }
        else{
            return E_FAIL;
        }
    }
    //IStream
    virtual HRESULT __stdcall SetSize(ULARGE_INTEGER) override{
        return E_NOTIMPL;
    }
    virtual HRESULT __stdcall Clone(IStream **i) override{
        return E_NOTIMPL;
    }
    virtual HRESULT __stdcall Seek(LARGE_INTEGER offset,DWORD flags,ULARGE_INTEGER *new_pos) override{
        int sdl_flags;
        switch(flags){
            case STREAM_SEEK_CUR:
                sdl_flags = RW_SEEK_CUR;
                break;
            case STREAM_SEEK_SET:
                sdl_flags = RW_SEEK_SET;
                break;
            case STREAM_SEEK_END:
                sdl_flags = RW_SEEK_END;
                break;
        }
        if(SDL_RWseek(rwops,offset.QuadPart,sdl_flags) < 0){
            return E_FAIL;
        }
        if(new_pos != nullptr){
            new_pos->QuadPart = SDL_RWtell(rwops);
        }
        return S_OK;
    }

    virtual HRESULT __stdcall Commit(DWORD) override{
        return E_NOTIMPL;
    }
    //Ignored
    virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override{
        return E_NOTIMPL;
    }
    virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override{
        return E_NOTIMPL;
    }
    virtual HRESULT __stdcall Stat(STATSTG *stat,DWORD) override{
        //Must impl
        auto now = SDL_RWtell(rwops);
        //Get end
        SDL_RWseek(rwops,0,RW_SEEK_END);
        auto end = SDL_RWtell(rwops);
        stat->cbSize.QuadPart = end - cur;
        SDL_RWseek(rwops,now,RW_SEEK_SET);
        return S_OK;
    }
    virtual HRESULT __stdcall CopyTo(IStream*,ULARGE_INTEGER,ULARGE_INTEGER*,ULARGE_INTEGER*) override{
        return E_NOTIMPL;
    }
    virtual HRESULT __stdcall Revert() override{
        return E_NOTIMPL;
    }
    //RwopsIStream
    RwopsIstream(SDL_RWops *r){
        rwops = r;
        //Get current
        cur = SDL_RWtell(r);
    }
    ~RwopsIstream(){
        SDL_RWseek(rwops,cur,RW_SEEK_SET);
    }
    SDL_RWops *rwops;
    Sint64 cur;
    ULONG refcount = 1;
};
}
namespace{
    #define WIC_CHECK(HR) if(FAILED(HR)) {\
        SDL_SetError("WIC Error %ld",HR); \
        return ;\
    }
    #define WIC_CHECK2(HR,RET) if(FAILED(HR)) {\
        SDL_SetError("WIC Error %ld",HR); \
        return RET;\
    }
    static IWICImagingFactory *wic_factory = nullptr;


    template <class T>
    using ComInstance = Btk::Win32::ComInstance<T>;

    void cleanup_wic(){
        wic_factory->Release();
        CoUninitialize();
    }
    bool InitWIC(){
        HRESULT hr;
        hr = CoInitializeEx(0,COINIT_MULTITHREADED);
        WIC_CHECK2(hr,false);
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void**>(&wic_factory)
        );
        if(FAILED(hr)){
            CoUninitialize();
        }
        Btk::AtExit(cleanup_wic);
        return true;
    }
    Uint32     translate_wic_fmt(const GUID &wic_fmt){
        return 0;
    }
    const GUID &translate_sdl_fmt(Uint32 fmt){
        switch(fmt){
            case SDL_PIXELFORMAT_RGBA32:
                return GUID_WICPixelFormat32bppPRGBA;
            default:
                return GUID_WICPixelFormatUndefined;
        }
    }
    SDL_Surface *load_image_istream(IStream *istream){
        HRESULT hr;
        ComInstance<IWICBitmapDecoder> decoder;
        ComInstance<IWICBitmapFrameDecode> frame;
        GUID frame_fmt;

        hr = wic_factory->CreateDecoderFromStream(
            istream,
            nullptr,
            WICDecodeMetadataCacheOnDemand,
            &decoder
        );
        WIC_CHECK2(hr,nullptr);
        //Create decoder done
        hr = decoder->GetFrame(0,&frame);
        WIC_CHECK2(hr,nullptr);
        //Get info
        UINT w,h;
        hr = frame->GetSize(&w,&h);
        hr = frame->GetPixelFormat(&frame_fmt);

        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,h,
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32),
            SDL_PIXELFORMAT_RGBA32
        );
        if(surf == nullptr){
            return nullptr;
        }
        if(frame_fmt == GUID_WICPixelFormat32bppPRGBA){
            //Same fmt, donnot need convert
            hr = frame->CopyPixels(
                nullptr,
                w * 4,
                w * h * 4,
                (BYTE*)surf->pixels
            );
        }
        else{
            //Convert it
            ComInstance<IWICFormatConverter> converter;
            hr = wic_factory->CreateFormatConverter(&converter);
            hr = converter->Initialize(
                frame,
                GUID_WICPixelFormat32bppPRGBA,
                WICBitmapDitherTypeNone,
                nullptr,
                0,
                WICBitmapPaletteTypeCustom
            );
            hr = converter->CopyPixels(
                nullptr,
                w * 4,
                w * h * 4,
                (BYTE*)surf->pixels
            );
        }
        //Done
        return surf;
    }
    //TODO
    #if 0
    bool wic_save_generic(const GUID &encoder_fmt,SDL_Surface *surf,SDL_RWops *rwops){
        //Check format
        const GUID &surf_fmt = translate_sdl_fmt(surf->format->format);
        if(surf_fmt == GUID_WICPixelFormatUndefined){
            //must convert
            SDL_Surface *new_surf = SDL_ConvertSurfaceFormat(
                surf,
                SDL_PIXELFORMAT_RGBA32,
                0
            );
            if(new_surf == nullptr){
                return false;
            }
            bool ret = wic_save_generic(encoder_fmt,new_surf,rwops);
            SDL_FreeSurface(new_surf);
            return ret;
        }

        ComInstance<IWICBitmapEncoder> encoder;
        ComInstance<IWICBitmapFrameEncode> frame;
        HRESULT hr;
        GUID fmt;
        int w,h;
        w = surf->w;
        h = surf->h;

        hr = wic_factory->CreateEncoder(
            encoder_fmt,
            nullptr,
            &encoder
        );
        WIC_CHECK2(hr,false);
        //Wrap SDL_RWops
        RwopsIstream stream(rwops);
        hr = encoder->Initialize(
            &stream,
            WICBitmapEncoderNoCache
        );
        WIC_CHECK2(hr,false);
        hr = encoder->GetContainerFormat(&fmt);
        WIC_CHECK2(hr,false);
        hr = encoder->CreateNewFrame(&frame,nullptr);
        WIC_CHECK2(hr,false);
        //Init
        hr = frame->Initialize(nullptr);
        hr = frame->SetSize(w,h);
        //Write pixels
        ComInstance<IWICFormatConverter> converter;
        hr = wic_factory->CreateFormatConverter(&converter);
        // hr = converter->Initialize(
        //     frame,
        //     GUID_WICPixelFormat32bppPRGBA,
        //     WICBitmapDitherTypeNone,
        //     nullptr,
        //     0,
        //     WICBitmapPaletteTypeCustom
        // );
        hr = converter->CopyPixels(
            nullptr,
            w * 4,
            w * h * 4,
            (BYTE*)surf->pixels
        );
        //copy to frame
        frame->WriteSource(converter,nullptr);


        //flush
        hr = frame->Commit();
        hr = encoder->Commit();
        return true;
    }
    #endif
    // SDL_Surface *load_image_wic_direct(SDL_RWops *rwops){
    //     BTK_RW_SAVE_STATUS(rwops);
    //     Sint64 size = Btk::RWtellsize(rwops);
    //     //Alloc mem,and read into it
    //     HGLOBAL memhandle = GlobalAlloc(GMEM_MOVEABLE,size);
    //     if(memhandle == INVALID_HANDLE_VALUE){
    //         return nullptr;
    //     }
    //     //Load data begin
    //     void *addr = GlobalLock(memhandle);
    //     if(SDL_RWread(rwops,addr,1,size) != size){
    //         //Errpr
    //         GlobalUnlock(memhandle);
    //         CloseHandle(memhandle);
    //         return nullptr;
    //     }
    //     GlobalUnlock(memhandle);

    //     //Load data end
    //     ComInstance<IStream> stream;
    //     if(FAILED(CreateStreamOnHGlobal(memhandle,TRUE,&stream))){
    //         //FAILED
    //         CloseHandle(memhandle);
    //         return nullptr;
    //     }
    //     return load_image_istream(stream);
    // }
    SDL_Surface *load_image_wic(SDL_RWops *rwops){
        RwopsIstream stream(rwops);
        return load_image_istream(&stream);
    }
}

#define BTK_WIC_WRAPPER(TYPE) {\
    ImageAdapter adapter;\
    adapter.name = #TYPE;\
    adapter.vendor = "wincodec";\
    adapter.fn_load = load_image_wic;\
    RegisterImageAdapter(adapter);\
}

namespace Btk{
    void RegisterWIC(){
        if(InitWIC()){
            BTK_WIC_WRAPPER(JPEG);
            BTK_WIC_WRAPPER(TIFF);
            BTK_WIC_WRAPPER(DDS);
            BTK_WIC_WRAPPER(BMP);
            BTK_WIC_WRAPPER(GIF);
            BTK_WIC_WRAPPER(ICO);
            BTK_WIC_WRAPPER(PNG);
        }
    }
}