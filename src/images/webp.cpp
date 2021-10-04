#include "../build.hpp"
#include "./adapter.hpp"

#include <webp/decode.h>
#include <webp/encode.h>

#include <Btk/impl/scope.hpp>
#include <Btk/impl/codec.hpp>
#include <Btk/exception.hpp>

namespace{
    const char *tr_error_by_vp8(VP8StatusCode code){
        #ifndef BTK_WEBP_NOERRMSG
        #define TRANSLATE_VP8STATUS(E) \
            case VP8_STATUS_##E : errstring = "VP8_STATUS_" #E; break
        
        const char *errstring = nullptr;
        switch(code){
            TRANSLATE_VP8STATUS(OUT_OF_MEMORY);
            TRANSLATE_VP8STATUS(INVALID_PARAM);
            TRANSLATE_VP8STATUS(BITSTREAM_ERROR);
            TRANSLATE_VP8STATUS(UNSUPPORTED_FEATURE);
            TRANSLATE_VP8STATUS(SUSPENDED);
            TRANSLATE_VP8STATUS(USER_ABORT);
            TRANSLATE_VP8STATUS(NOT_ENOUGH_DATA);
            default:
                //Impossible
                {abort();}
        }
        return errstring;
        #undef TRANSLATE_VP8STATUS
        #else
        return "InternalError";
        #endif
    }
    SDL_Surface *load_webp(SDL_RWops *rwops){
        WebPDecoderConfig config;
        WebPInitDecoderConfig(&config);

        WebPDecBuffer &buffer = config.output;
        WebPBitstreamFeatures &stream = config.input;

        //Begin config
        buffer.colorspace = MODE_RGBA;

        //Read all data
        size_t buffersize;
        void *bufferdata = SDL_LoadFile_RW(rwops,&buffersize,SDL_FALSE);
        if(bufferdata == nullptr){
            //Read Error
            return nullptr;
        }
        Btk::SDLScopePtr _bufferdata(bufferdata);
        //Begin parse
        auto code = WebPDecode(static_cast<Uint8*>(bufferdata),buffersize,&config);
        if(code != VP8_STATUS_OK){
            //Failed
            SDL_SetError("WebP %d => %s",code,tr_error_by_vp8(code));
            return nullptr;
        }
        SDL_Surface *surf = SDL_CreateRGBSurfaceWithFormat(
            0,
            buffer.width,
            buffer.height,
            SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32),
            SDL_PIXELFORMAT_RGBA32
        );
        if(surf == nullptr){
            //Create surface failed
            WebPFreeDecBuffer(&buffer);
            return nullptr;
        }
        //Copy data
        memcpy(surf->pixels,buffer.u.RGBA.rgba,buffer.u.RGBA.size);
        //Cleanup
        WebPFreeDecBuffer(&buffer);
        return surf;
    }
    int webp_write(const uint8_t* data, size_t data_size,
                   const WebPPicture* picture){
        return SDL_RWwrite(
            static_cast<SDL_RWops*>(picture->custom_ptr),
            data,
            sizeof(char),
            data_size
        );
    }
    bool save_webp(SDL_RWops *rwops,SDL_Surface *surf,int quality){
        //Check format
        if(surf->format->format != SDL_PIXELFORMAT_ARGB32){
            SDL_Surface *surface = SDL_ConvertSurfaceFormat(
                surf,
                SDL_PIXELFORMAT_ARGB32,
                0
            );
            if(surface == nullptr){
                return false;
            }
            Btk::PixBuf buf(surface);
            return save_webp(rwops,surface,quality);
        }
        //Make config
        WebPConfig config;
        WebPConfigInit(&config);

        //Make sure is lossless
        config.lossless = 1;

        //Make pic
        WebPPicture picture;
        WebPPictureInit(&picture);
        picture.use_argb = 1;
        picture.width  = surf->w;
        picture.height = surf->h;
        WebPPictureAlloc(&picture);
        //Alloc mem
        BTK_LOCKSURFACE(surf);
        WebPPictureImportRGBA(
            &picture,
            static_cast<Uint8*>(surf->pixels),
            surf->pitch
        );
        BTK_UNLOCKSURFACE(surf);
        //Set callback
        picture.custom_ptr = rwops;
        picture.writer = webp_write;
        //Begin write
        int val = WebPEncode(&config,&picture);
        WebPPictureFree(&picture);

        return val == 0;
    }

    using Btk::throwRuntimeError;
    using Btk::ImageDecoder;
    using Btk::PixelFormat;
    using Btk::Rect;
    using Btk::Size;
    //TODO
    struct WebPDecoder:public ImageDecoder{
        //Data readed
        Uint8 *bitstream;
        size_t datalen;


        WebPBitstreamFeatures features;
    };
}

namespace Btk{
    void RegisterWEBP(){
        ImageAdapter adapter;
        adapter.name = "webp";
        adapter.vendor = "libwebp";
        adapter.fn_load = load_webp;
        adapter.fn_save = save_webp;
        adapter.fn_is = BultinIsWebP;
        RegisterImageAdapter(adapter);
    }
}