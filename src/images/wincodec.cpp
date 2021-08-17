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
        ++refcount;
        return refcount;
    }
    virtual ULONG __stdcall Release() override{
        --refcount;
        auto r = refcount;
        if(refcount == 0){
            delete this;
        }
        return r;
    }
    virtual HRESULT __stdcall QueryInterface(REFIID,void**) override{
        return -1;
    }
    //ISequentialStream
    virtual HRESULT __stdcall Read(void *buf, ULONG bufsize, ULONG *readed) override{
        size_t ret = SDL_RWread(rwops,buf,1,bufsize);
        *readed = ret;
        if(ret == bufsize){
            return 1;
        }
        else{
            return -1;
        }
    }
    virtual HRESULT __stdcall Write(const void *buf,ULONG bufsize,ULONG *writed) override{
        size_t ret = SDL_RWwrite(rwops,buf,1,bufsize);
        *writed = ret;
        if (ret == bufsize){
            return 1;
        }
        else{
            return -1;
        }
    }
    //IStream
    virtual HRESULT __stdcall SetSize(ULARGE_INTEGER) override{
        return 1;
    }
    virtual HRESULT __stdcall Clone(IStream **i) override{
        *i = new RwopsIstream(rwops);
        return 1;
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
        new_pos->QuadPart = SDL_RWseek(rwops,offset.QuadPart,sdl_flags);
        if(new_pos < 0){
            return -1;
        }
        return 1;
    }

    virtual HRESULT __stdcall Commit(DWORD) override{
        return S_OK;
    }
    //Ignored
    virtual HRESULT __stdcall LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD) override{
        return -1;
    }
    virtual HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override{
        return -1;
    }
    virtual HRESULT __stdcall Stat(STATSTG *,DWORD) override{
        return -1;
    }
    virtual HRESULT __stdcall CopyTo(IStream*,ULARGE_INTEGER,ULARGE_INTEGER*,ULARGE_INTEGER*) override{
        return -1;
    }
    virtual HRESULT __stdcall Revert() override{
        return S_OK;
    }
    //RwopsIStream
    RwopsIstream(SDL_RWops *r){
        rwops = r;
    }
    SDL_RWops *rwops;
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
    }
    void InitWIC(){
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wic_factory)
        );
        WIC_CHECK(hr);
        Btk::AtExit(cleanup_wic);
    }
    Uint32 translate_wic_fmt(const GUID &wic_fmt){
        return 0;
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
    SDL_Surface *load_image_wic_direct(SDL_RWops *rwops){
        size_t bufsize;
        void * buf = SDL_LoadFile_RW(rwops,&bufsize,SDL_FALSE);
        if(buf == nullptr){
            return nullptr;
        }
        Btk::SDLScopePtr sdlptr(buf);
        //Load data end
        ComInstance<IStream> stream(SHCreateMemStream((BYTE*)buf,bufsize));
        return load_image_istream(stream);
    }
    SDL_Surface *load_image_wic(SDL_RWops *rwops){
        RwopsIstream stream(rwops);
        return load_image_istream(&stream);
    }
}

#define BTK_WIC_WRAPPER(TYPE) {\
    ImageAdapter adapter;\
    adapter.name = #TYPE;\
    adapter.vendor = "wic";\
    adapter.fn_load = load_image_wic_direct;\
    RegisterImageAdapter(adapter);\
}

namespace Btk{
    void RegisterWIC(){
        BTK_WIC_WRAPPER(JPEG);
        BTK_WIC_WRAPPER(TIFF);
        BTK_WIC_WRAPPER(DDS);
        BTK_WIC_WRAPPER(BMP);
        BTK_WIC_WRAPPER(GIF);
        BTK_WIC_WRAPPER(ICO);
        BTK_WIC_WRAPPER(PNG);
    }
}