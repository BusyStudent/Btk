#include "../build.hpp"
#include "adapter.hpp"
#include "../platform/win32/internal.hpp"

#include <Btk/Btk.hpp>

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
        //TODO Add Convertion
        //*new_pos = SDL_RWseek(rwops,offset,sdl_flags);
        return -1;
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
        if(FAILED(hr)){
            return;
        }
        Btk::AtExit(cleanup_wic);
    }
}