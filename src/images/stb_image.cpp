
#include "../build.hpp"
#include "./adapter.hpp"

#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_rwops.h>
#include <Btk/detail/codec.hpp>
#include <Btk/exception.hpp>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC

#define STBI_ASSERT(X) BTK_ASSERT(X)
#define STBI_MALLOC SDL_malloc
#define STBI_REALLOC SDL_realloc
#define STBI_FREE SDL_free

//STB WRITE CONFIGURE
#ifndef BTK_STBI_NOWRITE

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC

#define STBIW_ASSERT(X) BTK_ASSERT(X)
#define STBIW_MALLOC SDL_malloc
#define STBIW_REALLOC SDL_realloc
#define STBIW_FREE SDL_free

//Use zlib instead of it
#ifdef BTK_STBIMAGE_USE_ZLIB
#define STBIW_ZLIB_COMPRESS zlib_compress
#include <zlib.h>
static uint8_t *zlib_compress(uint8_t *data, int data_len, int *out_len, int quality){
    uLongf dst_len = ::compressBound(data_len);
    uint8_t *buf = (uint8_t*)STBIW_MALLOC(dst_len * sizeof(uint8_t));
    if(buf == nullptr){
        return nullptr;
    }
    //call zlib
    if(::compress2(buf,&dst_len,data,data_len,quality) != Z_OK){
        STBIW_FREE(buf);
        return nullptr;
    }
    *out_len = dst_len;
    return buf;
}

#endif

#endif


extern "C"{
    #include "../libs/stb_image.h"
    //Enable write by default
    #ifndef BTK_STBI_NOWRITE
    #include "../libs/stb_image_write.h"
    #endif
}

//Make alias jpg -> jpeg
#define stbi__jpg_test stbi__jpeg_test
//Make check functions for STB IMAGE
#define BTK_STBI_IS(TYPE) stb_is<stbi__##TYPE##_test>
#define BTK_STBI_SAVE(TYPE) stb_write_helper<stbi__##TYPE##_test>::write

#define BTK_STBI_WRAPPER(TYPE) {\
        ImageAdapter adapter;\
        adapter.vendor = "stbi";\
        adapter.name = #TYPE;\
        adapter.fn_load = stbi_load;\
        adapter.fn_save = BTK_STBI_SAVE(TYPE);\
        adapter.fn_is = BTK_STBI_IS(TYPE);\
        RegisterImageAdapter(adapter);\
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define STBI_RGB SDL_PIXELFORMAT_RGB888
#else
    #define STBI_RGB SDL_PIXELFORMAT_BGR888
#endif


namespace{
    //TODO ADD Error check
    //Callback wrapper for stb_callback
    struct BTKHIDDEN stbi_io_cb:public stbi_io_callbacks{
        stbi_io_cb(SDL_RWops *rwops){
            read = [](void *user,char *data,int size){
                return static_cast<stbi_io_cb*>(user)->_read(data,size);
            };
            eof = [](void *user){
                return static_cast<stbi_io_cb*>(user)->_eof();
            };
            skip = [](void *user,int n){
                return static_cast<stbi_io_cb*>(user)->_skip(n);
            };
            fptr = rwops;
            //Set current position
            cur = SDL_RWtell(fptr);
            //Seek to end
            SDL_RWseek(fptr,0,RW_SEEK_END);
            //Get end
            end = SDL_RWtell(fptr);
            //Reset
            SDL_RWseek(fptr,cur,RW_SEEK_SET);
            BTK_LOGINFO("[STBII]Current pos %d",int(cur));
        }
        int _read(char *data,int size){
            size_t ret = SDL_RWread(fptr,data,sizeof(char),size);
            // BTK_LOGINFO("Read %d byte",int(ret));
            return ret;
        }
        int _eof(){
            //Is eof
            BTK_LOGINFO("[STBII]Got eof");
            return SDL_RWtell(fptr) == end;
        }
        void _skip(int n){
            //Seek at cur
            BTK_LOGINFO("[STBII]Skip %d byte");
            SDL_RWseek(fptr,n,RW_SEEK_CUR);
        }
        /**
         * @brief Reset the stream status to prev
         * 
         */
        void _reset_to_prev(){
            BTK_LOGINFO("[STBII]Reset to Prev");
            SDL_RWseek(fptr,cur,RW_SEEK_SET);
            BTK_LOGINFO("[STBII]After reset,Current pos %d",int(SDL_RWtell(fptr)));
        }
        SDL_RWops *fptr;
        Sint64 cur;//Current pos
        Sint64 end;//The end position
    };
    SDL_Surface *stbi_from_rgba(
        stbi_uc *data,
        int w,
        int h){
        //Slow way
        #if 0
        //Create surface RGBA32
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,h,
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32),
            SDL_PIXELFORMAT_RGBA32
        );
        if(surf == nullptr){
            stbi_image_free(data);
            Btk::throwSDLError();
        }
        //Then Update it
        memcpy(surf->pixels,data,w * h * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32));
        stbi_image_free(data);
        
        #else
        //Faster way,but more dangerous
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
            static_cast<void*>(data),
            w,h,
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32),
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32) * w,
            SDL_PIXELFORMAT_RGBA32
        );
        if(surf == nullptr){
            stbi_image_free(data);
            Btk::throwSDLError();
        }
        //Now set the flags to let SDL free the memory itselfs
        //Acrodding to this https://twitter.com/icculus/status/667036586610139137
        surf->flags ^= SDL_PREALLOC;
        #endif
        return surf;
    }
    SDL_Surface *stbi_from_rgb(
        stbi_uc *data,
        int w,
        int h){
        //FIXME It will cause a render error when format is RGB888
        //Probably ENDIAN
        
        //BGR888
        #if 0
        //Slower
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,h,
            SDL_BYTESPERPIXEL(STBI_RGB),
            STBI_RGB
        );
        if(surf == nullptr){
            stbi_image_free(data);
            Btk::throwSDLError();
        }
        //Then Update it
        memcpy(surf->pixels,data,w * h * SDL_BYTESPERPIXEL(STBI_RGB));
        stbi_image_free(data);
        #else
        //Faster
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormatFrom(
            static_cast<void*>(data),
            w,h,
            SDL_BYTESPERPIXEL(STBI_RGB),
            SDL_BYTESPERPIXEL(STBI_RGB) * w,
            STBI_RGB
        );
        if(surf == nullptr){
            stbi_image_free(data);
            Btk::throwSDLError();
        }
        surf->flags ^= SDL_PREALLOC;
        #endif
        return surf;
    }
    SDL_Surface *stbi_from_grey(
        stbi_uc *data,
        int w,
        int h){

        //Begin convert and copy
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            w,h,
            SDL_BYTESPERPIXEL(STBI_RGB),
            STBI_RGB
        );
        if(surf == nullptr){
            stbi_image_free(data);
            Btk::throwSDLError();
        }
        Uint8 *pixels = static_cast<Uint8*>(surf->pixels);

        for(int y = 0;y < surf->h;y++){
            for(int x = 0;x < surf->w;x++){
                pixels[y * surf->w + x] = SDL_MapRGB(
                    surf->format,
                    data[y * surf->w + x],
                    data[y * surf->w + x],
                    data[y * surf->w + x]
                );
            }
        }


        stbi_image_free(data);
        return nullptr;
    }
    SDL_Surface *stbi_from_greya(
        stbi_uc *data,
        int w,
        int h){
        
        //TODO
        stbi_image_free(data);
        return nullptr;
    }
    /**
     * @brief Create a sdl surface by pixels
     * 
     * @param data The pixels data(The fuction will manage it)
     * @param w 
     * @param h 
     * @param comp 
     * @return SDL_Surface* 
     */
    SDL_Surface *stbi_make_surf(
        stbi_uc *data,
        int w,
        int h,
        int comp){

        switch(comp){
            case STBI_rgb_alpha:{
                return stbi_from_rgba(data,w,h);
            }
            case STBI_rgb:{
                return stbi_from_rgb(data,w,h);
            }
            case STBI_grey_alpha:{
                return stbi_from_greya(data,w,h);
            }
            case STBI_grey:{
                return stbi_from_grey(data,w,h);
            }
            default:
                //Impossible
                BTK_LOGDEBUG("[STBII]Internal Error in stbi_make_surf");
                std::abort();
        }
    }
    SDL_Surface *stbi_load(SDL_RWops *rwops){
        SDL_Surface *surf;
        stbi_io_cb callback(rwops);
        int w,h;
        int comp;
        
        stbi_uc *data = stbi_load_from_callbacks(&callback,&callback,&w,&h,&comp,STBI_rgb_alpha);
        if(data == nullptr){
            SDL_SetError("%s",stbi_failure_reason());
            return nullptr;
        }
        BTK_LOGINFO("[STBII] recevie comp %d",comp);

        return stbi_make_surf(data,w,h,comp);
    }
    /**
     * @brief Helper template to check type
     * 
     * @tparam STBCHECK 
     * @param rwops 
     * @return true 
     * @return false 
     */
    template<auto STBCHECK>
    bool stb_is(SDL_RWops *rwops){
        stbi_io_cb callback(rwops);
        stbi__context s;
        stbi__start_callbacks(&s,(stbi_io_callbacks *)&callback,&callback);
        bool value = STBCHECK(&s);
        callback._reset_to_prev();
        return value;
        
    }
    /**
     * @brief Helper class for generate output function
     * 
     * @tparam T 
     */
    template<auto T>
    struct stb_write_helper{
        static constexpr auto write = nullptr;
    };
    #ifndef BTK_STBI_NOWRITE
    struct stb_write_base{
        stb_write_base(SDL_RWops *r):rwops(r){};
        SDL_RWops *rwops;
        bool ok = true;

        /**
         * @brief Check pixel format
         * 
         * @param callable Callable(SDL_Surface *surf,int comp)
         */
        template<class Callable>
        bool do_write(Callable &&callable,SDL_Surface *surf){

            SDL_Surface *target;
            int comp;
            //Check format
            if(surf->format->format == SDL_PIXELFORMAT_RGBA32){
                //RGBA32
                comp = STBI_rgb_alpha;
                target = surf;
            }
            else if(surf->format->format == STBI_RGB){
                comp = STBI_rgb;
                target = surf;
            }
            else{
                //Convert to RGBA32
                target = SDL_ConvertSurfaceFormat(surf,SDL_PIXELFORMAT_RGBA32,0);
                comp = STBI_rgb_alpha;
                if(target == nullptr){
                    //Convert failed
                    return false;
                }
            }
            //Call it
            if(SDL_MUSTLOCK(target)){
                SDL_LockSurface(target);
            }
            ok = callable(target,comp);
            if(SDL_MUSTLOCK(target)){
                SDL_UnlockSurface(target);
            }
            if(target != surf){
                //Is converted surface
                SDL_FreeSurface(target);
            }
            return ok;
        }
    };
    //stb write callback
    static void stbwrite_callback(void *context, void *data, int size){
        auto base = static_cast<stb_write_base*>(context);
        size_t ret = SDL_RWwrite(base->rwops,data,sizeof(char),size);
        if(ret != size){
            base->ok = false;
        }
    }
    template<>
    struct stb_write_helper<stbi__png_test>{
        static bool write(SDL_RWops *r,SDL_Surface *surface,int){
            stb_write_base base(r);
            //Forward to stb
            auto callback = [&](SDL_Surface *surf,int comp){
                return stbi_write_png_to_func(
                    stbwrite_callback,
                    &base,
                    surf->w,
                    surf->h,
                    comp,
                    surf->pixels,
                    // surf->w * comp,
                    surf->pitch
                );
            };
            return base.do_write(callback,surface);
        }
    };
    template<>
    struct stb_write_helper<stbi__jpeg_test>{
        static bool write(SDL_RWops *r,SDL_Surface *surface,int quality){
            stb_write_base base(r);
            //Forward to stb
            auto callback = [&](SDL_Surface *surf,int comp){
                return stbi_write_jpg_to_func(
                    stbwrite_callback,
                    &base,
                    surf->w,
                    surf->h,
                    comp,
                    surf->pixels,
                    quality
                );
            };
            return base.do_write(callback,surface);
        }
    };
    //Show we provide it,Btk had already provide a BMP Adapter
    template<>
    struct stb_write_helper<stbi__bmp_test>{
        static bool write(SDL_RWops *r,SDL_Surface *surface,int){
            stb_write_base base(r);
            //Forward to stb
            auto callback = [&](SDL_Surface *surf,int comp){
                return stbi_write_bmp_to_func(
                    stbwrite_callback,
                    &base,
                    surf->w,
                    surf->h,
                    comp,
                    surf->pixels
                );
            };
            return base.do_write(callback,surface);
        }
    };
    template<>
    struct stb_write_helper<stbi__tga_test>{
        static bool write(SDL_RWops *r,SDL_Surface *surface,int){
            stb_write_base base(r);
            //Forward to stb
            auto callback = [&](SDL_Surface *surf,int comp){
                return stbi_write_tga_to_func(
                    stbwrite_callback,
                    &base,
                    surf->w,
                    surf->h,
                    comp,
                    surf->pixels
                );
            };
            return base.do_write(callback,surface);
        }
    };
    template<>
    struct stb_write_helper<stbi__hdr_test>{
        static bool write(SDL_RWops *r,SDL_Surface *surface,int){
            stb_write_base base(r);
            //Forward to stb
            auto callback = [&](SDL_Surface *surf,int comp){
                if(surf->format->BytesPerPixel != sizeof(float)){
                    Btk::throwRuntimeError("Unsupported");
                }
                return stbi_write_hdr_to_func(
                    stbwrite_callback,
                    &base,
                    surf->w,
                    surf->h,
                    comp,
                    static_cast<float*>(surf->pixels)
                );
            };
            return base.do_write(callback,surface);
        }
    };
    #endif
}

namespace{
    using Btk::throwRuntimeError;
    using Btk::throwSDLError;
    using Btk::ImageDecoder;
    using Btk::PixelFormat;
    using Btk::Rect;
    using Btk::Size;
    //STB Gif decoder
    struct BTKHIDDEN STBGifDecoder:public ImageDecoder{
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
            if(data == nullptr){
                throwRuntimeError("Decoder is not opened");
            }
        }
        //Image information
        int w = 0;
        int h = 0;
        int z = 0;
        int comp = 0;
        int *delays = nullptr;
        stbi_uc *data = 0;
    };
    void STBGifDecoder::decoder_open(){
        if(data != nullptr){
            decoder_close();
        }
        //Read data from stream
        size_t bufferlen;
        void *buffer;
        buffer = SDL_LoadFile_RW(stream(),&bufferlen,SDL_FALSE);
        if(buffer == nullptr){
            throwSDLError();
        }
        data = stbi_load_gif_from_memory(
            static_cast<stbi_uc*>(buffer),
            bufferlen,
            &delays,
            &w,
            &h,
            &z,
            &comp,
            STBI_rgb_alpha
        );
        SDL_free(buffer);
        //Parse failed
        if(data == nullptr){
            throwRuntimeError(stbi_failure_reason());
        }
        //TODO Handle Gray and Gray Alpha
        if(comp != STBI_rgb_alpha and comp != STBI_rgb){
            BTK_UNIMPLEMENTED();
        }
        //Done
    }
    void STBGifDecoder::decoder_close(){
        STBI_FREE(delays);
        STBI_FREE(data);

        delays = nullptr;
        data = nullptr;
    }
    void STBGifDecoder::query_info(
        size_t *p_n_frame,
        PixelFormat *p_fmt){
        check_opened();
        if(p_n_frame != nullptr){
            *p_n_frame = z;
        }
        if(p_fmt != nullptr){
            if(comp == STBI_rgb_alpha){
                *p_fmt = PixelFormat::RGBA32;
            }
            else if(comp == STBI_rgb){
                *p_fmt = STBI_RGB;
            }
            else{
                BTK_UNIMPLEMENTED();
            }
        }
    }
    void STBGifDecoder::query_frame(
        size_t frame_index,
        Size *p_size,
        int *p_delay
    ){
        check_opened();
        if(frame_index > z){
            throwRuntimeError("Out of range");
        }
        if(p_size != nullptr){
            p_size->w = w;
            p_size->h = h;
        }
        if(p_delay != nullptr){
            *p_delay = delays[frame_index];
        }
    }
    void STBGifDecoder::read_pixels(
        size_t frame_index,
        const Rect *rect,
        void *pixels,
        const PixelFormat *wanted){
        //TODO
        //Do check
        check_opened();
        if(frame_index > z){
            throwRuntimeError("Out of range");
        }
        
        BTK_ASSERT(rect == nullptr);
        BTK_ASSERT(wanted == nullptr);
        //Get current frame
        int stride_bytes = comp * w;
        stbi_uc *frame = data + stride_bytes * h * frame_index;

        //Do copy
        memcpy(pixels,frame,w * h * comp);
    }
}

namespace Btk{
    void RegisterSTBII(){
        stbi_set_unpremultiply_on_load(1);
        stbi_convert_iphone_png_to_rgb(1);
        
        BTK_STBI_WRAPPER(jpeg);        
        BTK_STBI_WRAPPER(jpg);//JPEG alias

        BTK_STBI_WRAPPER(png);        
        BTK_STBI_WRAPPER(tga);        
        BTK_STBI_WRAPPER(hdr);        
        BTK_STBI_WRAPPER(pic);        
        BTK_STBI_WRAPPER(bmp);
        BTK_STBI_WRAPPER(psd);
        BTK_STBI_WRAPPER(pnm);
        //Gif decoder

        ImageAdapter adapter;
        adapter.fn_is = BTK_STBI_IS(gif);
        adapter.fn_load = stbi_load;
        adapter.name = "gif";
        adapter.vendor = "stbi";
        adapter.create_decoder = []() -> ImageDecoder*{
            return new STBGifDecoder();
        };
        RegisterImageAdapter(adapter);
    }
}