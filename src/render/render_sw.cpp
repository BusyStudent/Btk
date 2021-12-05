#include "../build.hpp"

//Software render
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_hints.h>
#include <Btk/impl/utils.hpp>
#include <Btk/gl/software.hpp>
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
        set_backend(RendererBackend::Software);

        tag_surf = SDL_GetWindowSurface(win);
        tag_win = win;
        surf_owned = true;

    }
    SWDevice::SWDevice(SDL_Surface *surf,bool owned){
        set_backend(RendererBackend::Software);

        tag_surf = surf;
        surf_owned = owned;

    }
    SWDevice::~SWDevice(){
        if(surf_owned){
            SDL_FreeSurface(tag_surf);
        }
    }
    void SWDevice::clear_buffer(Color c){
        // SDL_FillRect(
        //     tag_surf,
        //     nullptr,
        //     SDL_MapRGBA(tag_surf->format,c.r,c.g,c.b,c.a)
        // );
        //Update current framebuffer
        update_status();
        if(current_ctxt != nullptr){
            nvgClearBackgroundRT(current_ctxt,c.r,c.g,c.b,c.a);
        }
        // Size size;
        // nvgFrameBufferSizeRT(current_ctxt,&size.w,&size.h);
        // Uint32 *pixels = reinterpret_cast<Uint32*>(
        //     nvgReadPixelsRT(current_ctxt)
        // );
        // for(int y = 0;y < size.h;y++){
        //     for(int x = 0;x < size.w;x++){
        //         pixels[y * size.w + x] = MapRGBA32(c);
        //     }
        // }
        
    }
    void SWDevice::swap_buffer(){
        if(current_ctxt == nullptr){
            return;
        }

        Size  fb_size;
        Size  sf_size(tag_surf->w,tag_surf->h);
        void *pixels = nvgReadPixelsRT(current_ctxt);
        nvgFrameBufferSizeRT(current_ctxt,&fb_size.w,&fb_size.h);

        // //Check the buffer and surface is the same size
        // if(fb_size != sf_size){
        //     //TODO Slove it
        //     BTK_LOGINFO("[SWDevice] Wrong Framebuffer size,swap_buffer() was canneled");
        //     return;
        // }
        
        // //Copy it to surface
        // if(SDL_MUSTLOCK(tag_surf)){
        //     SDL_LockSurface(tag_surf);
        // }

        // if(tag_fmt != PixelFormat::RGBA32){
        //     //We need cvt
        //     SDL_ConvertPixels(
        //         tag_surf->w,
        //         tag_surf->h,
        //         PixelFormat::RGBA32,
        //         pixels,
        //         tag_surf->w * SDL_BYTESPERPIXEL(PixelFormat::RGBA32),
        //         tag_surf->format->format,
        //         tag_surf->pixels,
        //         tag_surf->pitch
        //     );
        // }
        // else{
        //     //Just copy
        //     memcpy(
        //         tag_surf->pixels,
        //         pixels,
        //         tag_surf->pitch * tag_surf->h
        //     );
        // }

        // if(SDL_MUSTLOCK(tag_surf)){
        //     SDL_UnlockSurface(tag_surf);
        // }
        if(viewport.empty()){
            SDL_BlitSurface(framebuffer,nullptr,tag_surf,nullptr);
        }
        else{
            SDL_BlitSurface(framebuffer,nullptr,tag_surf,&viewport);
        }

        if(tag_win != nullptr){
            SDL_UpdateWindowSurface(tag_win);
        }
    }
    auto SWDevice::create_context() -> Context{
        if(current_ctxt == nullptr){
            current_ctxt = nvgCreateRT(
                NVG_ANTIALIAS | NVG_STENCIL_STROKES,
                tag_surf->w,
                tag_surf->h
            );
            return current_ctxt;
        }
        return nullptr;
    }
    void SWDevice::destroy_context(Context ctxt){
        nvgDeleteRT(ctxt);
        current_ctxt = nullptr;
    }
    bool SWDevice::output_size(
        Size *p_logical_size,
        Size *p_physical_size
    ){
        update_status();

        if(p_logical_size != nullptr){
            if(tag_win != nullptr){
                SDL_GetWindowSize(tag_win,&p_logical_size->w,&p_logical_size->h);
            }
            else{
                p_logical_size->w = tag_surf->w;
                p_logical_size->h = tag_surf->h;
            }
        }
        if(p_physical_size != nullptr){
            p_physical_size->w = tag_surf->w;
            p_physical_size->h = tag_surf->h;
        }
        return true;
    }
    void SWDevice::fb_resize(int new_w,int new_h){
        //Resize the framebuffer
        SDL_FreeSurface(framebuffer);
        nvgResizeFrameBufferRT(current_ctxt,new_w,new_h);
        framebuffer = SDL_CreateRGBSurfaceWithFormatFrom(
            nvgReadPixelsRT(current_ctxt),
            new_w,
            new_h,
            SDL_BYTESPERPIXEL(PixelFormat::RGBA32),
            new_w * SDL_BYTESPERPIXEL(PixelFormat::RGBA32),
            PixelFormat::RGBA32
        );
    }
    void SWDevice::update_status(){
        if(not frame_begined and tag_win != nullptr){
            Size win_size;
            Size surf_size(tag_surf->w,tag_surf->h);
            SDL_GetWindowSize(tag_win,&win_size.w,&win_size.h);
            if(win_size != surf_size){
                //Recreate surface
                SDL_FreeSurface(tag_surf);
                tag_surf = SDL_GetWindowSurface(tag_win);
            }
            if(current_ctxt != nullptr){
                fb_resize(win_size.w,win_size.h);
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
        
        update_status();
        RendererDevice::begin_frame(ctxt,w,h,pixel_ratio);
        frame_begined = true;
    }
    void SWDevice::end_frame(Context ctxt){
        RendererDevice::end_frame(ctxt);
        frame_begined = false;
        update_status();
    }
    void SWDevice::set_viewport(const Rect *rect){
        if(rect == nullptr){
            viewport = {
                -1,
                -1,
                -1,
                -1
            };
        }
        else{
            viewport = *rect;
        }
    }
    //Target IS IGNORED
    void SWDevice::set_target(Context,TextureID){}
    void SWDevice::reset_target(Context){}

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