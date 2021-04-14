#include "../build.hpp"

#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_timer.h>
#include <Btk/exception.hpp>
#include <Btk/imageview.hpp>
#include <Btk/render.hpp>
#include <Btk/pixels.hpp>
#include <Btk/event.hpp>
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
        if(index == 0){
            update_frame(index,buf);
        }
        else{
            update_frame(index - 1,buf);
            update_frame(index,buf);
        }
        return buf;
    }
    void GifImage::update_frame(size_t index,PixBuf &buf,int *ms) const{
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

        ExtensionBlock *ext = nullptr;
        //Ext
        for(int i = 0;i < image.ExtensionBlockCount;i++){
            ExtensionBlock &block = image.ExtensionBlocks[i];
            if(block.Function == GRAPHICS_EXT_FUNC_CODE){
                ext = &block;
            }
        }
        //Delay time
        if(ms != nullptr){
            if(ext != nullptr){
                *ms = (10 *((ext->Bytes[2]) << 8 | (ext->Bytes[1])));
            }
            else{
                //We donnot known
                *ms = -1;
            }
        }
        int w,h;
        int top,left;

        w = image.ImageDesc.Width;
        h = image.ImageDesc.Height;
        top = image.ImageDesc.Top;
        left = image.ImageDesc.Left;

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
        //Pixelbuf's x and y
        int buf_x = 0;
        int buf_y = 0;
        for(int i = top;i < h + top;i++){
            for(int l = left;l < w + left;l++){
                //Foreach pixels
                int bit = image.RasterBits[i * w + l];
                
                if(ext != nullptr){
                    //Check the graphics control
                    if(bit ==  ext-> Bytes[3] && ext->Bytes[0]){
                        //We should skip it
                        buf_x ++;
                        continue;
                    }
                }
                
                GifColorType color = map->Colors[bit];
                pixels[buf_y * w + buf_x] = SDL_MapRGB(format,color.Red,color.Green,color.Blue);
                buf_x ++;
            }
            buf_x = 0;
            buf_y ++;
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
namespace Btk{
    GifView::GifView() = default;
    GifView::GifView(int x,int y,int w,int h){
        attr.user_rect = true;
        set_rect(x,y,w,h);
    }
    GifView::~GifView() = default;
    bool GifView::handle(Event &event){
        switch(event.type()){
            case Event::SetRect:
                rect = event_cast<SetRectEvent&>(event).rect();
                return event.accept();
            case Event::SetContainer:
                parent = event_cast<SetContainerEvent&>(event).container();
                return event.accept();
            default:
                return event.reject();
        }
    }
    void GifView::set_image(GifImage &&image){
        gifimage = std::move(image);
        auto [w,h] = gifimage.size();
        frame = PixBuf(w,h,SDL_PIXELFORMAT_RGBA32);
        
        cur_frame = 0;
        delete_texture = true;

        redraw();
    }
    void GifView::draw(Renderer &render){
        //Refresh the frame
        if(delete_texture){
            texture.clear();
            delete_texture = false;
        }
        if(frame.empty()){
            return;
        }
        if(texture.empty()){
            texture = render.create_from(frame);
        }
        //update
        int ms = 0;
        gifimage.update_frame(cur_frame,frame,&ms);
        texture.update(frame);

        render.draw_image(texture,rect);


        cur_frame ++;
        if(cur_frame >= gifimage.image_count()){
            cur_frame = 0;
        }
        //It should be improved
        //TODO:Add timer
        SDL_Delay(ms);
        redraw();
    }
}