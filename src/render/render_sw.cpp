#include "../build.hpp"

//Software render
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_render.h>
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
inline constexpr int NVG_IMAGE_NODELETE			= 1<<16;	// Do not delete GL texture handle.


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
        }
        ~SWContext();
        //For nvg interface
        int  create();
        int  create_texture();
        int  delete_texture(int image);
        int  update_texture(int image,int x,int y,int w,int h,const Uint8* data);
        int  get_texture_size(int image,int* w,int* h);
        void viewport();
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
            TextureFlags flags;
            Uint32 format;
            int w,h;
        };
        SDL_Renderer *sdl_render;
        //Map int -> SDLTexture
        std::map<TextureID,SDLTexture> texs_map;
        //ID Generator
        std::mt19937 random;
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
        //Update the texture
        return SDL_UpdateTexture(
            texture->tex,
            &r,
            static_cast<const void *>(data),
            SDL_BYTESPERPIXEL(texture->format) * w
        ) == 0;
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
        NVGparams params;
        SWContext *sw = new SWContext(r);
        params.userPtr = sw;
        params.renderDelete = [](void *uptr){
            delete static_cast<SWContext*>(uptr);
        };
        params.renderDeleteTexture = [](void *uptr,int image){
            return static_cast<SWContext*>(uptr)->delete_texture(image);
        };
        params.renderUpdateTexture = [](void* uptr,int image,int x,int y,int w,int h,const Uint8* data){
            return static_cast<SWContext*>(uptr)->update_texture(image,x,y,w,h,data);
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
        bool owned_render = true;
    };

    SWDevice::~SWDevice(){
        if(owned_render){
            SDL_DestroyRenderer(renderer);
        }
        if(owned_surf){
            SDL_FreeSurface(data.surf);
        }
    }
    void SWDevice::swap_buffer(){
        SDL_RenderPresent(renderer);
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
