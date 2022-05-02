#include "../build.hpp"

//Software render
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>
#include <Btk/detail/utils.hpp>
#include <Btk/graphics/software.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <cassert>
#include <random>
#include <limits>
#include <vector>
#include <map>


#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include <queue>
#include <bitset>

#include <algorithm>
#include <functional>


namespace{
    #define NANOVG_RT_IMPLEMENTATION
    #define NANORT_IMPLEMENTATION
    #include "../libs/nanovg_rt.h"
}

// // These are additional flags on top of NVGimageFlags.
// inline constexpr int NVG_IMAGE_NODELETE			= 1<<16;	// Do not delete SDL texture handle.
namespace Btk{
    SWDevice::SWDevice(SDL_Window *win){
        //TODO Still has bug on not has hw_bilt
        set_backend(RendererBackend::Software);

        window = win;

        //Try to use HW_Bilt
        using RendererInfo = std::pair<int,SDL_RendererInfo>;
        int num_dev = SDL_GetNumRenderDrivers();
        std::vector<RendererInfo> infos_vec;

        SDL_RendererInfo info;

        for(int i = 0;i < num_dev;i++){
            SDL_GetRenderDriverInfo(i,&info);
            if((info.flags & SDL_RENDERER_ACCELERATED) != SDL_RENDERER_ACCELERATED){
                continue;
            }
            for(int n = 0;n < info.num_texture_formats;n++){
                if(info.texture_formats[n] == SDL_PIXELFORMAT_RGBA32){
                    //Support RGBA32
                    infos_vec.emplace_back(i,info);
                    continue;
                }
            }
        }
        //Sort by scores
        std::qsort(
            infos_vec.data(),
            infos_vec.size(),
            sizeof(RendererInfo),
            [](const void *_a,const void *_b) -> int{ 
                auto a = static_cast<const RendererInfo*>(_a);
                auto b = static_cast<const RendererInfo*>(_b);
                int na = (a->second.flags & SDL_RENDERER_PRESENTVSYNC) == SDL_RENDERER_PRESENTVSYNC;
                int nb = (b->second.flags & SDL_RENDERER_PRESENTVSYNC) == SDL_RENDERER_PRESENTVSYNC;
                return na > nb;
            }
        );
        //Try create the renderer
        for(int n = 0;n < infos_vec.size();n ++){
            int idx = infos_vec[n].first;
            hw_render = SDL_CreateRenderer(window,idx,infos_vec[n].second.flags);
            if(hw_render == nullptr){
                continue;
            }
            BTK_LOGINFO("[SWDevice]hw_bilt backend %s",infos_vec[n].second.name);
            //Try create texture
            int out_w,out_h,pitch;
            void *tpix;
            SDL_GetWindowSize(window,&out_w,&out_h);
            hw_framebuffer = SDL_CreateTexture(
                hw_render,
                SDL_PIXELFORMAT_RGBA32,
                SDL_TEXTUREACCESS_STREAMING,
                out_w,
                out_h
            );
            if(hw_framebuffer == nullptr){
                //create failed
                goto err;
            }
            
            #if 1
            //Try to test lock it
            if(SDL_LockTexture(hw_framebuffer,nullptr,&tpix,&pitch) == -1){
                goto err;
            }
            SDL_UnlockTexture(hw_framebuffer);
            if(pitch != out_w * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32)){
                goto err;
            }
            #endif

            //Done
            hw_bilt = true;
            hw_owned = true;
            return;
            //Handle Error
            err:
                SDL_DestroyTexture(hw_framebuffer);
                SDL_DestroyRenderer(hw_render);
                hw_framebuffer = nullptr;
                hw_render = nullptr;
                continue;
        }
        //No HW_Bilt usable
        window_framebuffer = SDL_GetWindowSurface(window);
        sw_owned = true;
    }
    SWDevice::SWDevice(SDL_Surface *surf,bool owned){
        set_backend(RendererBackend::Software);
        sw_framebuffer = surf;
        sw_owned = owned;
        window_framebuffer = SDL_GetWindowSurface(window);
    }
    SWDevice::~SWDevice(){
        if(sw_owned){
            SDL_FreeSurface(sw_framebuffer);
        }
        if(hw_bilt and hw_owned){
            SDL_DestroyTexture(hw_framebuffer);
            SDL_DestroyRenderer(hw_render);
        }
        if(not hw_bilt){
            SDL_FreeSurface(window_framebuffer);
        }
    }
    void SWDevice::clear_buffer(Color c){
        if(hw_bilt and not hw_attached){
            void *pixels;
            int   pitch;
            int   w,h;
            SDL_LockTexture(hw_framebuffer,nullptr,&pixels,&pitch);
            SDL_GetWindowSize(window,&w,&h);
            nvgAttachFrameBufferRT(sw_context,pixels,w,h);
            hw_attached = true;
        }
        nvgClearBackgroundRT(sw_context,c.r,c.g,c.b,c.a);
        if(hw_bilt){
            SDL_SetRenderDrawColor(hw_render,c.r,c.g,c.b,c.a);
            SDL_RenderClear(hw_render);
        }
    }
    void SWDevice::swap_buffer(){
        if(sw_context == nullptr){
            return;
        }
        if(window != nullptr){
            if(hw_bilt){
                SDL_RenderCopy(hw_render,hw_framebuffer,nullptr,nullptr);
                SDL_RenderPresent(hw_render);
            }
            else{
                SDL_BlitSurface(sw_framebuffer,nullptr,window_framebuffer,nullptr);
                SDL_UpdateWindowSurface(window);
            }
        }
    }
    auto SWDevice::create_context() -> Context{
        if(sw_context == nullptr){
            if(hw_bilt){
                int out_w,out_h;
                SDL_GetRendererOutputSize(hw_render,&out_w,&out_h);
                sw_context = nvgCreateRT(
                    NVG_ANTIALIAS | NVG_STENCIL_STROKES,
                    out_w,out_h
                );

            }
            else{
                sw_context = nvgCreateRT(
                    NVG_ANTIALIAS | NVG_STENCIL_STROKES,
                    sw_framebuffer->w,
                    sw_framebuffer->h
                );
            }
            return sw_context;
        }
        return nullptr;
    }
    void SWDevice::destroy_context(Context ctxt){
        nvgDeleteRT(ctxt);
        sw_context = nullptr;
    }
    bool SWDevice::output_size(
        Size *p_logical_size,
        Size *p_physical_size
    ){
        if(p_logical_size != nullptr){
            if(window != nullptr){
                SDL_GetWindowSize(window,&(p_logical_size->w),&(p_logical_size->h));
            }
            else{
                p_logical_size->w = sw_framebuffer->w;
                p_logical_size->h = sw_framebuffer->h;
            }
        }
        if(p_physical_size != nullptr){
            if(window != nullptr){
                SDL_GetWindowSize(window,&(p_physical_size->w),&(p_physical_size->h));
            }
            else{
                p_physical_size->w = sw_framebuffer->w;
                p_physical_size->h = sw_framebuffer->h;
            }
        }
        return true;
    }
    void SWDevice::sw_resize(int new_w,int new_h){
        //Resize the framebuffer
        SDL_FreeSurface(sw_framebuffer);
        nvgResizeFrameBufferRT(sw_context,new_w,new_h);
        sw_framebuffer = SDL_CreateRGBSurfaceWithFormatFrom(
            nvgReadPixelsRT(sw_context),
            new_w,
            new_h,
            SDL_BYTESPERPIXEL(PixelFormat::RGBA32),
            new_w * SDL_BYTESPERPIXEL(PixelFormat::RGBA32),
            PixelFormat::RGBA32
        );
    }
    void SWDevice::active_env(){
        if(window != nullptr){
            if(hw_bilt){
                //Has SDL_Renderer for HW Bilt
                int fb_w,fb_h;
                int ofb_w,ofb_h;
                SDL_GetWindowSize(window,&fb_w,&fb_h);
                SDL_QueryTexture(hw_framebuffer,nullptr,nullptr,&ofb_w,&ofb_h);
                if(ofb_w != fb_w or ofb_h != fb_h){
                    //Small framebuffer
                    SDL_DestroyTexture(hw_framebuffer);
                    hw_framebuffer = SDL_CreateTexture(
                        hw_render,
                        SDL_PIXELFORMAT_RGBA32,
                        SDL_TEXTUREACCESS_STREAMING,
                        fb_w,
                        fb_h
                    );
                }
            }
            else{
                Size win_size;
                Size surf_size(window_framebuffer->w,window_framebuffer->h);
                SDL_GetWindowSize(window,&win_size.w,&win_size.h);
                if(win_size != surf_size){
                    //Recreate surface
                    SDL_FreeSurface(window_framebuffer);
                    window_framebuffer = SDL_GetWindowSurface(window);
                }
                if(sw_context != nullptr){
                    sw_resize(win_size.w,win_size.h);
                }
            }
        }
    }
    void *SWDevice::lock_texture(Context ctxt,TextureID id,const Rect *r,LockFlag){
        //Only support when r == nullptr
        if(r != nullptr){
            auto tex = rtnvg__findTexture(
                static_cast<RTNVGcontext*>(nvgInternalParams(ctxt)->userPtr),
                id
            );
            if(tex == nullptr){
                return nullptr;
            }
            return tex->data;

        }
        return nullptr;
    }
    void  SWDevice::unlock_texture(Context ,TextureID ,void *){
        //Do no-op
    }
    //FB
    void SWDevice::begin_frame(
        Context ctxt,
        float w,
        float h,
        float pixel_ratio){
        
        active_env();

        if(hw_bilt and not hw_attached){
            void *pixels;
            int   pitch;
            SDL_LockTexture(hw_framebuffer,nullptr,&pixels,&pitch);
            nvgAttachFrameBufferRT(ctxt,pixels,w,h);
            hw_attached = true;
        }

        RendererDevice::begin_frame(ctxt,w,h,pixel_ratio);
    }
    void SWDevice::end_frame(Context ctxt){
        RendererDevice::end_frame(ctxt);
        if(hw_bilt and hw_attached){
            hw_attached = false;
            SDL_UnlockTexture(hw_framebuffer);
        }
        active_env();
        
    }
    void SWDevice::set_viewport(const Rect *rect){
        // if(rect == nullptr){
        //     viewport = {
        //         -1,
        //         -1,
        //         -1,
        //         -1
        //     };
        // }
        // else{
        //     viewport = *rect;
        // }
    }
    //Target IS IGNORED
    void SWDevice::set_target(Context,TextureID){

    }
    void SWDevice::reset_target(Context){

    }

    bool SWDevice::query_texture(
        Context ctxt,
        TextureID id,
        Size *p_size,
        void *p_handle,
        TextureFlags *p_flags){

        auto tex = rtnvg__findTexture(
            static_cast<RTNVGcontext*>(nvgInternalParams(ctxt)->userPtr),
            id
        );
        if(tex == nullptr){
            return false;
        }
        if(p_size != nullptr){
            p_size->w = tex->width;
            p_size->h = tex->height;
        }
        if(p_handle != nullptr){
            *static_cast<void**>(p_handle) = tex->data;
        }
        if(p_flags != nullptr){
            *p_flags = TextureFlags(tex->flags);
        }
        return true;
    }
    TextureID SWDevice::clone_texture(Context ctxt,TextureID id){
        auto tex = rtnvg__findTexture(
            static_cast<RTNVGcontext*>(nvgInternalParams(ctxt)->userPtr),
            id
        );
        if(tex == nullptr){
            return -1;
        }
        return create_texture(
            ctxt,
            tex->width,
            tex->height,
            TextureFlags(tex->flags),
            tex->data
        );
    }
}