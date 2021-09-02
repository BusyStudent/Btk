#include "../build.hpp"

//Software render
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>
#include <Btk/impl/utils.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <cassert>
#include <random>
#include <limits>
#include <map>
extern "C"{
    #include "../libs/nanovg.h"
}
// These are additional flags on top of NVGimageFlags.
inline constexpr int NVG_IMAGE_NODELETE			= 1<<16;	// Do not delete SDL texture handle.


#if SDL_VERSION_ATLEAST(2,0,10)
    //Support float
    #define BTK_SW_FLOAT 1
    #define BTK_SW_RECT  SDL_FRect
    #define BTK_SW_POINT SDL_FPoint
    #define BTK_SW_WRAPPER(NAME) static auto constexpr NAME = SDL_Render##NAME##F;
#else
    //Unsupport float
    #define BTK_SW_FLOAT 0
    #define BTK_SW_RECT  SDL_Rect
    #define BTK_SW_POINT SDL_Point
    #define BTK_SW_WRAPPER(NAME) static auto constexpr NAME = SDL_Render##NAME;
#endif



#define BTK_SW_FIND(ID,PTEXTURE,RET) \
    if(auto iter = texs_map.find(ID);iter == texs_map.end()){ \
        return RET;\
    }\
    else{ \
        PTEXTURE = &(iter->second);\
    }

namespace Btk{
    //TODO 
    /**
     * @brief Nanovg software renderer context based on SDL_Renderer
     * 
     */
    struct BTKHIDDEN SWContext{
        //SDL Renderer rect and point
        using Rect = BTK_SW_RECT;
        using Point = BTK_SW_POINT;

        SWContext(SDL_Renderer *r){
            sdl_render = r;
            random.seed(std::random_device()());
            //Get hint from 
            hint_linear = SDL_GetHintBoolean("SDL_RENDER_SCALE_QUALITY",SDL_FALSE);
        }
        ~SWContext();
        //For nvg interface
        int  create();
        int  create_texture(int type,int w,int h,int imageFlags,const Uint8* data);
        int  delete_texture(int image);
        int  update_texture(int image,int x,int y,int w,int h,const Uint8* data);
        int  get_texture_size(int image,int* w,int* h);
        void viewport(float w,float h,float ratio);
        void cancel();
        void flush();
        void fill();
        void stroke();
        void triangles();

        //SDL renderer tiny wrapper
        BTK_SW_WRAPPER(DrawPoint);
        BTK_SW_WRAPPER(DrawPoints);
        BTK_SW_WRAPPER(FillRect);
        BTK_SW_WRAPPER(FillRects);
        BTK_SW_WRAPPER(DrawRect);
        BTK_SW_WRAPPER(DrawRects);
        BTK_SW_WRAPPER(Copy);
        BTK_SW_WRAPPER(CopyEx);

        //Internal 
        TextureID alloc_tex();

        static NVGcontext *Create(SDL_Renderer *r);

        struct SDLTexture{
            SDL_Texture *tex;
            NVGtexture   type;//NVG Texture type
            TextureFlags flags;
            Uint32 format;
            int w,h;
        };
        SDL_Renderer *sdl_render;
        //Map int -> SDLTexture
        std::map<TextureID,SDLTexture> texs_map;
        //ID Generator
        std::mt19937 random;
        SDL_bool hint_linear;
    };
    //Internal begin
    TextureID SWContext::alloc_tex(){
        TextureID id;
        std::uniform_int_distribution<TextureID> dist(0,std::numeric_limits<TextureID>::max());
        do{
            id = dist(random);
        }while(texs_map.find(id) != texs_map.end());
        //Generate a unique id
        texs_map[id];
        return id;
    }
    int SWContext::update_texture(int image,int x,int y,int w,int h,const Uint8* data){
        SDLTexture *texture;
        BTK_SW_FIND(image,texture,false);

        SDL_Rect r = {w,y,w,h};
        if(texture->type == NVG_TEXTURE_ALPHA){
            //RGBA32 Texture copy each pixel in chnnal
            void *_pixels;
            int   pitch;
            if(SDL_LockTexture(texture->tex,&r,&_pixels,&pitch) != 0){
                return false;
            }
            Uint32 *pixels = static_cast<Uint32*>(_pixels);

            int n = 0;
            for(int x = 0;x < r.w;x++){
                for(int y = 0;y < r.h;y ++){
                    pixels[y * r.w + x] = MapRGBA32(
                        data[n],
                        data[n],
                        data[n]
                    );
                    n += 1;
                }
                n += 1;
            }
            SDL_UnlockTexture(texture->tex);
            return true;
        }

        //Update the texture
        return SDL_UpdateTexture(
            texture->tex,
            &r,
            static_cast<const void *>(data),
            SDL_BYTESPERPIXEL(texture->format) * w
        ) == 0;
    }
    #if SDL_VERSION_ATLEAST(2,0,10)
    static void tr_scale_mode(TextureFlags &f,SDL_ScaleMode m){
        switch(m){
            case SDL_ScaleModeBest:
            case SDL_ScaleModeLinear:
                f |= TextureFlags::Linear;
                break;
            case SDL_ScaleModeNearest:
                f |= TextureFlags::Nearest;
        }
    }
    #endif
    int SWContext::create_texture(int type,int w,int h,int imageFlags,const Uint8* data){
        //Do check
        if(w <= 0 or h <= 0){
            return -1;
        }
        TextureID id = alloc_tex();
        SDLTexture &texture = texs_map[id];

        texture.h = h;
        texture.w = w;
        texture.type = NVGtexture(type);
        texture.flags = TextureFlags(imageFlags);
        texture.format = PixelFormat::RGBA32;
        texture.tex = SDL_CreateTexture(
            sdl_render,
            texture.format,
            SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET,
            w,h
        );
        if(texture.tex == nullptr){
            //Create err
            delete_texture(id);
            return -1;
        }
        //Update data if needed
        if(data != nullptr){
            update_texture(id,0,0,w,h,data);
        }
        //Check flags

        //Liner and nerest only support after 2.0.10
        #if SDL_VERSION_ATLEAST(2,0,10)
        //Current mode
        SDL_ScaleMode scale_mode;
        SDL_GetTextureScaleMode(texture.tex,&scale_mode);
        if((texture.flags & TextureFlags::Nearest) == TextureFlags::Nearest){
            //Nearest
            if(scale_mode != SDL_ScaleModeNearest){
                if(SDL_SetTextureScaleMode(texture.tex,SDL_ScaleModeNearest) != 0){
                    //failed
                    tr_scale_mode(texture.flags,scale_mode);
                }
            }
        }
        else{
            //Liner
            if(scale_mode == SDL_ScaleModeNearest){
                if(SDL_SetTextureScaleMode(texture.tex,SDL_ScaleModeLinear) != 0){
                    //failed
                    tr_scale_mode(texture.flags,scale_mode);
                }
            }
        }
        #else
        //Remove the flags
        if(hint_linear){
            //Default is linear
            texture.flags |= TextureFlags::Linear;
        }
        else{
            texture.flags |= TextureFlags::Nearest;
        }
        #endif
        //Unsupport flags
        texture.flags ^= TextureFlags::GenerateMipmaps;
        texture.flags ^= TextureFlags::RepeatX;
        texture.flags ^= TextureFlags::RepeatY;
        texture.flags ^= TextureFlags::Premultiplied;

        return id;
    }
    int SWContext::delete_texture(int image){
        auto iter = texs_map.find(image);
        if(iter == texs_map.end()){
            return false;
        }
        if((int(iter->second.flags) & NVG_IMAGE_NODELETE) != NVG_IMAGE_NODELETE){
            SDL_DestroyTexture(iter->second.tex);
        }
        texs_map.erase(iter);
        return true;
    }
    int SWContext::get_texture_size(int image,int *w,int *h){
        SDLTexture *texture;
        BTK_SW_FIND(image,texture,false);
        if(w != nullptr){
            *w = texture->w;
        }
        if(h != nullptr){
            *h = texture->h;
        }
        return true;
    }
    //Viewport
    void SWContext::viewport(float w,float h,float ratio){
        //SDL_RenderSetLogicalSize(sdl_render,w,h);
        int out_w,out_h;
        SDL_GetRendererOutputSize(sdl_render,&out_w,&out_h);
        float w_ratio,h_ratio;
        w_ratio = float(out_w) / w;
        h_ratio = float(out_h) / h;
        SDL_RenderSetScale(sdl_render,w_ratio,h_ratio);
    }
    SWContext::~SWContext(){
        //Destroy all texture
        for(auto iter = texs_map.begin();iter != texs_map.end();){
            if((int(iter->second.flags) & NVG_IMAGE_NODELETE) != NVG_IMAGE_NODELETE){
                //We need to delete the texture
                SDL_DestroyTexture(iter->second.tex);
            }
            iter = texs_map.erase(iter);
        }
    }
    //Internal end
    NVGcontext *SWContext::Create(SDL_Renderer *r){
        NVGparams params = {};
        SWContext *sw = new SWContext(r);
        params.userPtr = sw;
        params.renderDelete = [](void *uptr){
            delete static_cast<SWContext*>(uptr);
        };
        params.renderCreateTexture = [](void *uptr,int type,int w,int h,int imageFlags,const Uint8* data){
            return static_cast<SWContext*>(uptr)->create_texture(type,w,h,imageFlags,data);
        };
        params.renderDeleteTexture = [](void *uptr,int image){
            return static_cast<SWContext*>(uptr)->delete_texture(image);
        };
        params.renderUpdateTexture = [](void* uptr,int image,int x,int y,int w,int h,const Uint8* data){
            return static_cast<SWContext*>(uptr)->update_texture(image,x,y,w,h,data);
        };
        params.renderViewport = [](void *uptr,float w,float h,float ratio){
            return static_cast<SWContext*>(uptr)->viewport(w,h,ratio);
        };
        return nvgCreateInternal(&params);
    }

    struct SWDevice:public RendererDevice{
        SWDevice(SDL_Surface  *surf,bool owned = false);
        SWDevice(SDL_Renderer *render,bool owned = true);
        SWDevice(SDL_Window  *win);
        ~SWDevice();
        //Context
        Context create_context() override;
        void    destroy_context(Context) override;
        //Buffer
        void clear_buffer(Color bg) override;
        void swap_buffer() override;
        bool output_size(
            Size *p_logical_size,
            Size *p_physical_size
        ) override;
        //Viewport
        void set_viewport(const Rect *r) override;
        //Target
        void set_target(Context ctxt,TextureID id) override;
        void reset_target(Context ctxt) override;
        //Texture
        TextureID clone_texture(Context ctxt,TextureID) override;
        bool      query_texture(Context ctxt,
                                TextureID id,
                                Size *p_size,
                                void *p_handle,
                                TextureFlags *p_flags) override;


        union {
            SDL_Window *win;
            SDL_Surface *surf;
        }data;
        SDL_Renderer *renderer;
        //Free SDL_Surface
        bool owned_surf = false;
        bool owned_render = false;
        bool at_window = true;//Render At window
    };
    SWDevice::SWDevice(SDL_Window *win){
        renderer = SDL_CreateRenderer(
            win,
            -1,
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE
        );
        if(renderer == nullptr){
            throwRendererError();
        }
        data.win = win;
        owned_render = true;
        at_window = true;
    }
    SWDevice::SWDevice(SDL_Surface *surf,bool owned){
        renderer = SDL_CreateSoftwareRenderer(surf);
        if(renderer == nullptr){
            throwRendererError();
        }
        owned_render = true;
        owned_surf = owned;
        at_window = false;
        data.surf = surf;
    }
    SWDevice::~SWDevice(){
        if(owned_render){
            SDL_DestroyRenderer(renderer);
        }
        if(owned_surf){
            SDL_FreeSurface(data.surf);
        }
    }
    //Buffer
    void SWDevice::clear_buffer(Color c){
        SDL_SetRenderDrawColor(
            renderer,
            c.r,
            c.g,
            c.b,
            c.a
        );
        SDL_RenderClear(renderer);
    }
    void SWDevice::swap_buffer(){
        SDL_RenderPresent(renderer);
    }
    void SWDevice::set_viewport(const Rect *r){
        SDL_RenderSetViewport(renderer,r);
    }
    bool SWDevice::output_size(
            Size *p_logical_size,
            Size *p_physical_size){
        if(p_logical_size != nullptr){
            //Query logical size
            if(at_window){
                //Query window size
                SDL_GetWindowSize(
                    data.win,
                    &(p_logical_size->w),
                    &(p_logical_size->h)
                );
            }
            else{
                //Query surface
                p_logical_size->w = data.surf->w;
                p_logical_size->h = data.surf->h;
            }
        }
        if(p_physical_size != nullptr){
            return SDL_GetRendererOutputSize(
                renderer,
                &(p_logical_size->w),
                &(p_logical_size->h)
            ) == 0;
        }
        return true;
    }
    //Context
    NVGcontext *SWDevice::create_context(){
        return SWContext::Create(renderer);
    }
    void        SWDevice::destroy_context(Context ctxt){
        nvgDeleteInternal(ctxt);
    }
}


//To make more friendly for user,Export it at CAPI
extern "C"{
    BTKAPI
    Btk::RendererDevice *Btk_CreateSWDevice(void *pixels,
                                            int width,
                                            int height,
                                            int depth,
                                            int pitch,
                                            Uint32 Rmask,
                                            Uint32 Gmask,
                                            Uint32 Bmask,
                                            Uint32 Amask){
        SDL_Surface *surf = SDL_CreateRGBSurfaceFrom(
            pixels,
            width,
            height,
            depth,
            pitch,
            Rmask,
            Gmask,
            Bmask,
            Amask
        );
        if(surf == nullptr){
            return nullptr;
        }
        return new Btk::SWDevice(surf,true);
    }
}
