#include <SDL2/SDL.h>
#ifdef BTK_HAS_SDLIMG
    #include <SDL2/SDL_image.h>
#endif
#ifdef BTK_USE_GFX
    #include <Btk/thirdparty/SDL2_rotozoom.h>
#endif

#include "../build.hpp"

#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/pixels.hpp>
#include <Btk/rwops.hpp>
#include <Btk/rect.hpp>
#include <ostream>
namespace Btk{
    PixBuf::~PixBuf(){
        SDL_FreeSurface(surf);
    }
    PixBuf::PixBuf(u8string_view file){
        surf = LoadImage(RWops::FromFile(file.data(),"rb").get());
        if(surf == nullptr){
            throwSDLError();
        }
    }
    PixBuf::PixBuf(int w,int h,Uint32 fmt){
        surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,
            h,
            SDL_BYTESPERPIXEL(fmt),
            fmt
        );
        if(surf == nullptr){
            throwSDLError();
        }
    }
    PixBuf PixBuf::ref() const{
        surf->refcount ++;
        return PixBuf(surf);
    }
    PixBuf PixBuf::clone() const{
        SDL_Surface *surf = SDL_DuplicateSurface(this->surf);
        if(surf == nullptr){
            throwSDLError(SDL_GetError());
        }
        else{
            return PixBuf(surf);
        }
    }
    //Lock
    void PixBuf::lock() const{
        if(not must_lock()){
            return;
        }
        if(SDL_LockSurface(surf) != 0){
            throwSDLError();
        }
    }
    void PixBuf::unlock() const noexcept{
        if(not must_lock()){
            return;
        }
        SDL_UnlockSurface(surf);
    }
    void PixBuf::set_rle(bool val){
        if(SDL_SetSurfaceRLE(surf,val) == -1){
            throwSDLError();
        }
    }
    //save PixBuf
    void PixBuf::save_bmp(RWops &rw){
        if(SDL_SaveBMP_RW(surf,rw.get(),false) == -1){
            throwSDLError();
        }
    }
    // void PixBuf::save_jpg(RWops &rw,int quality){
    //     if(IMG_SaveJPG_RW(surf,rw.get(),false,quality) == -1){
    //         throwSDLError();
    //     }
    // }
    // void PixBuf::save_png(RWops &rw,int quality){
    //     if(IMG_SaveJPG_RW(surf,rw.get(),false,quality) == -1){
    //         throwSDLError();
    //     }
    // }

    void PixBuf::save_bmp(u8string_view fname){
        auto rw = RWops::FromFile(fname.data(),"wb");
        PixBuf::save_bmp(rw);
    }
    // void PixBuf::save_jpg(u8string_view fname,int quality){
    //     auto rw = RWops::FromFile(fname.data(),"wb");
    //     PixBuf::save_jpg(rw,quality);
    // }
    // void PixBuf::save_png(u8string_view fname,int quality){
    //     auto rw = RWops::FromFile(fname.data(),"wb");
    //     PixBuf::save_png(rw,quality);
    // }
    //operators
    PixBuf &PixBuf::operator =(SDL_Surface *sf){
        SDL_FreeSurface(surf);
        surf = sf;
        return *this;
    }
    PixBuf &PixBuf::operator =(PixBuf &&sf){
        SDL_FreeSurface(surf);
        surf = sf.surf;
        sf.surf = nullptr;
        return *this;
    }
    PixBuf PixBuf::convert(Uint32 fmt) const{
        SDL_Surface *surf = SDL_ConvertSurfaceFormat(this->surf,fmt,0);
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
    }
    PixBuf PixBuf::zoom(double w_f,double_t h_f){
        SDL_Surface *surf = zoomSurface(this->surf,w_f,h_f,SMOOTHING_ON);
        if(surf == nullptr){
            throwSDLError();
        }
        return surf;
    }
    void PixBuf::bilt(const PixBuf &buf,const Rect *src,Rect *dst){
        if(SDL_BlitSurface(buf.get(),src,surf,dst) != 0){
            throwSDLError();
        }
    }
    void PixBuf::begin_mut(){
        if(empty()){
            return;
        }
        if(surf->refcount != 1){
            //Copy it and unref
            SDL_Surface *new_surf = SDL_DuplicateSurface(
                surf
            );
            if(new_surf == nullptr){
                throwSDLError();
            }
            SDL_FreeSurface(surf);
            surf = new_surf;
        }
    }
    //static method
    PixBuf PixBuf::FromMem(const void *mem,size_t size){
        auto rw = RWops::FromMem(mem,size);
        return FromRWops(rw);
    }
    PixBuf PixBuf::FromFile(u8string_view file){
        return PixBuf(file);
    }
    PixBuf PixBuf::FromFile(FILE *f){
        auto rw = RWops::FromFP(f,false);
        return FromRWops(rw);
    }
    PixBuf PixBuf::FromXPMArray(const char *const*da){
        #ifdef BTK_HAS_SDLIMG
        SDL_Surface *surf = IMG_ReadXPMFromArray(const_cast<char**>(da));
        if(surf == nullptr){
            throwSDLError(IMG_GetError());
        }
        return PixBuf(surf);
        #else
        SDL_Unsupported();
        throwSDLError();
        #endif
    }
    PixBuf PixBuf::FromRWops(RWops &rwops){
        SDL_Surface *surf = LoadImage(rwops.get());
        if(surf == nullptr){
            throwSDLError();
        }
        return PixBuf(surf);
    }
}
namespace Btk{
    //PixelFormat
    PixFmt::PixFmt(Uint32 pixfmt){
        fmt = SDL_AllocFormat(pixfmt);
        if(fmt == nullptr){
            throwSDLError();
        }
    }
    PixFmt::~PixFmt(){
        SDL_FreeFormat(fmt);
    }
    //Map RGB
    Uint32 PixFmt::map_rgb (Uint8 r,Uint8 g,Uint8 b) const{
        return SDL_MapRGB(fmt,r,g,b);
    }
    Uint32 PixFmt::map_rgba(Uint8 r,Uint8 g,Uint8 b,Uint8 a) const{
        return SDL_MapRGBA(fmt,r,g,b,a);
    }
    //Get names
    u8string_view PixFmt::name() const{
        return SDL_GetPixelFormatName(fmt->format);
    }
    std::ostream &operator <<(std::ostream &os,Color c){
        os << '(' << int(c.r) << ',' << int(c.g) << ',' << int(c.b) << ',' << int(c.a) << ')';
        return os;
    }
}