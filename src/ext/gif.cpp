#include "../build.hpp"

#include <SDL2/SDL_rwops.h>
#include <Btk/exception.hpp>
#include <Btk/pixels.hpp>
#include <Btk/rwops.hpp>
#include <gif_lib.h>

namespace{
    int input_wrapper(GifFileType *fp,GifByteType *data,int len){
        return SDL_RWread(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }
    int output_wrapper(GifFileType *fp,const GifByteType *data,int len){
        return SDL_RWwrite(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }

    inline GifFileType *get_gif(void *ptr){
        return static_cast<GifFileType*>(ptr);
    }
}
namespace Btk{
    GifImage::~GifImage(){
        DGifCloseFile(get_gif(pimpl),nullptr);
    }
    Size GifImage::size() const{
        return Size{
            get_gif(pimpl)->SWidth,
            get_gif(pimpl)->SHeight
        };
    }
    size_t GifImage::image_count() const{
        return get_gif(pimpl)->ImageCount;
    }
    PixBuf GifImage::get_image(size_t index) const{
        auto s = size();
        PixBuf buf(s.w,s.h,SDL_PIXELFORMAT_RGBA32);
        update_frame(index,buf);
        return buf;
    }
    void GifImage::update_frame(size_t index,PixBuf &buf) const{
        auto *fp = get_gif(pimpl);
        if(buf.empty()){
            return;
        }
        if(index > fp->ImageCount){
            throwRuntimeError("Out of range");
        }
        if(buf.size() != size()){
            throwRuntimeError("buf.size() != size()");
        }
        //Create the buf
        const SavedImage &image = fp->SavedImages[index];
        const ColorMapObject *map;
        int w,h;

        w = image.ImageDesc.Width;
        h = image.ImageDesc.Height;

        if(image.ImageDesc.ColorMap == nullptr){
            map = fp->SColorMap;
        }
        else{
            map = image.ImageDesc.ColorMap;
        }
        Uint32 *pixels = static_cast<Uint32*>(buf->pixels);;
        //ReadPixels
        SDL_PixelFormat *format = buf->format;
        if(buf.must_lock()){
            buf.lock();
        }
        for(int i = 0;i < h;i++){
            for(int l = 0;l < w;l++){
                int bit = image.RasterBits[i * w + l];
                GifColorType color = map->Colors[bit];
                pixels[i * w + l] = SDL_MapRGB(format,color.Red,color.Green,color.Blue);
            }
        }
        if(buf.must_lock()){
            buf.unlock();
        }
    }
    GifImage &GifImage::operator =(GifImage &&image){
        if(&image != this){
            DGifCloseFile(get_gif(pimpl),nullptr);
            pimpl = image.pimpl;
            image.pimpl = nullptr;
        }
        return *this;
    }
    GifImage GifImage::FromRwops(RWops &rwops){
        int error;
        auto *fp = DGifOpen(rwops.get(),input_wrapper,&error);
        if(fp == nullptr){
            //failed
            throwRuntimeError(GifErrorString(error));
        }
        error = DGifSlurp(fp);
        if(error == GIF_ERROR){
            DGifCloseFile(fp,nullptr);
            throwRuntimeError(GifErrorString(error));
        }
        return GifImage(fp);
    }
    GifImage GifImage::FromFile(std::string_view fname){
        auto rw = RWops::FromFile(fname.data(),"rb");
        return FromRwops(rw);
    }
}