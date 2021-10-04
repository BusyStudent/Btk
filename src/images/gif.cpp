#include "../build.hpp"
#include "adapter.hpp"


#include <SDL2/SDL_rwops.h>
#include <SDL2/SDL_timer.h>
#include <Btk/impl/codec.hpp>
#include <Btk/exception.hpp>
#include <Btk/imageview.hpp>
#include <Btk/render.hpp>
#include <Btk/pixels.hpp>
#include <Btk/event.hpp>
#include <Btk/rwops.hpp>
#include <gif_lib.h>

namespace{
    using Btk::throwRuntimeError;
    using Btk::ImageDecoder;
    using Btk::PixelFormat;
    using Btk::Rect;
    using Btk::Size;

    int input_wrapper(GifFileType *fp,GifByteType *data,int len){
        return SDL_RWread(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }
    int output_wrapper(GifFileType *fp,const GifByteType *data,int len){
        return SDL_RWwrite(static_cast<SDL_RWops*>(fp->UserData),data,1,len);
    }

    inline GifFileType *get_gif(void *ptr){
        return static_cast<GifFileType*>(ptr);
    }
    //Decoder
    struct BTKHIDDEN GiflibDecoder:public ImageDecoder{
        GifFileType *gif = nullptr;
        ExtensionBlock **blocks_array = nullptr;
        //< Array for Graphics control blocks
        SDL_PixelFormat *cfmt = nullptr;
        //< PixelFormat for convert
        void decoder_open() override;
        void decoder_close() override;
        void query_info(
            size_t *p_n_frame,
            PixelFormat *p_fmt
        ) override;
        void query_frame(
            size_t frame_index,
            Size *p_size,
            int *_delay
        ) override;
        void read_pixels(
            size_t frame_index,
            const Rect *rect,
            void *pixels,
            const PixelFormat *wanted
        ) override;

        void check_opened() const{
            if(gif == nullptr){
                throwRuntimeError("Decoder is not opened");
            }
        }
        inline
        GifColorType find_color(int cur_frame,int w,int h);
    };
    void GiflibDecoder::decoder_open(){
        if(gif != nullptr){
            //has opened already
            decoder_close();
        }
        //Try open
        int error;
        auto *fp = DGifOpen(stream(),input_wrapper,&error);
        if(fp == nullptr){
            //failed
            throwRuntimeError(GifErrorString(error));
        }
        error = DGifSlurp(fp);
        if(error == GIF_ERROR){
            DGifCloseFile(fp,nullptr);
            throwRuntimeError(GifErrorString(error));
        }
        gif = fp;
        //Open end

        //Begin query all graphics control blocks
        blocks_array = static_cast<ExtensionBlock**>(
            SDL_malloc(sizeof(ExtensionBlock*) * gif->ImageCount)
        );
        SDL_memset(blocks_array,0,sizeof(ExtensionBlock*) * gif->ImageCount);

        for(int i = 0;i < gif->ImageCount;i++){
            SavedImage &image = gif->SavedImages[i];
            for(int n = 0;n < image.ExtensionBlockCount;n++){
                ExtensionBlock &block = image.ExtensionBlocks[n];
                if(block.Function == GRAPHICS_EXT_FUNC_CODE){
                    //Founded
                    blocks_array[i] = &block;
                    break;
                }
            }
        }
        //Query end
    }
    void GiflibDecoder::decoder_close(){
        DGifCloseFile(gif,nullptr);
        SDL_FreeFormat(cfmt);
        SDL_free(blocks_array);
        gif = nullptr;
        cfmt = nullptr;
        blocks_array = nullptr;
    }
    void GiflibDecoder::query_info(
        size_t *p_n_frame,
        PixelFormat *p_fmt){
        check_opened();
        if(p_n_frame != nullptr){
            *p_n_frame = gif->ImageCount;
        }
        if(p_fmt != nullptr){
            *p_fmt = PixelFormat::RGBA32;
        }
    }
    void GiflibDecoder::query_frame(
        size_t frame_index,
        Size *p_size,
        int *p_delay){
        check_opened();
        if(frame_index > gif->ImageCount){
            throwRuntimeError("Out of range");
        }
        SavedImage &image = gif->SavedImages[frame_index];
        if(p_size != nullptr){
            //Get size
            p_size->w = image.ImageDesc.Width;
            p_size->h = image.ImageDesc.Height;
        }
        if(p_delay != nullptr){
            //query the ext blocks
            ExtensionBlock *ext = blocks_array[frame_index];
            if(ext == nullptr){
                //not founded
                *p_delay = -1;
            }
            else{
                //Get it
                *p_delay = (10 *((ext->Bytes[2]) << 8 | (ext->Bytes[1])));
            }
        }
    }
    void GiflibDecoder::read_pixels(
        size_t frame_index,
        const Rect *rect,
        void *pixels,
        const PixelFormat *_wanted
    ){
        //TODO
        check_opened();
        if(frame_index > gif->ImageCount){
            throwRuntimeError("Out of range");
        }
        //Check wanted format
        Uint32 fmt;
        if(_wanted == nullptr){
            fmt = PixelFormat::RGBA32;
        }
        else{
            fmt = *_wanted;
        }
        if(cfmt != nullptr){
            if(cfmt->format != fmt){
                //Realloc
                SDL_FreeFormat(cfmt);
                cfmt = SDL_AllocFormat(fmt);
            }
        }
        else{
            cfmt = SDL_AllocFormat(fmt);
        }


        BTK_ASSERT(rect == nullptr);

        SavedImage &image = gif->SavedImages[frame_index];
        int w = image.ImageDesc.Width;
        int h = image.ImageDesc.Height;

        Uint32 *buf = static_cast<Uint32*>(pixels);
        GifColorType color;
        for(int y = 0;y < h;y ++){
            for(int x = 0;x < w;x ++){
                color = find_color(frame_index,x,y);
                buf[y * w + x] = SDL_MapRGB(
                    cfmt,
                    color.Red,
                    color.Green,
                    color.Blue
                );
            }
        }
    }
    inline
    GifColorType GiflibDecoder::find_color(int cur_frame,int w,int h){
        for(int n = cur_frame;n >= 0;n --){
            ExtensionBlock *block = blocks_array[n];
            SavedImage &image = gif->SavedImages[n];
            int top,left;
            int width = image.ImageDesc.Width;
            top = image.ImageDesc.Top;
            left = image.ImageDesc.Left;

            int bit = image.RasterBits[(top + h) * width + (w + left)];
            //Get bit
            if(block != nullptr){
                //Check is skipped
                if(bit == block-> Bytes[3] && block->Bytes[0]){
                    //On no
                    //move to prev frame
                    continue;
                }
            }
            //Is the bit we wanted
            ColorMapObject *cmap;
            if(image.ImageDesc.ColorMap == nullptr){
                cmap = gif->SColorMap;
            }
            else{
                cmap = image.ImageDesc.ColorMap;
            }
            return cmap->Colors[bit];
        }
        //We could not find it
        return {0,0,0};
    }
}

namespace Btk{
    void RegisterGIF(){
        ImageAdapter adapter;
        adapter.name = "gif";
        adapter.vendor = "libgif";
        adapter.create_decoder = []() -> ImageDecoder *{
            return new GiflibDecoder;
        };
        adapter.fn_is = BultinIsGIF;
        adapter.fn_load = [](SDL_RWops *rwops) -> SDL_Surface *{
            std::unique_ptr<GiflibDecoder> decoder(new GiflibDecoder);
            decoder->open(rwops);

            auto [w,h] = decoder->frame_size(0);
            Uint32 fmt = decoder->container_format();
            SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
                0,
                w,h,
                SDL_BYTESPERPIXEL(fmt),
                fmt
            );
            decoder->read_pixels(0,nullptr,surf->pixels,nullptr);

            decoder->close();
            return surf;
        };
        RegisterImageAdapter(adapter);
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
                break;
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
    GifImage GifImage::FromFile(u8string_view fname){
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
        if(ms >= 0){
            SDL_Delay(ms);
        }
        redraw();
    }
}