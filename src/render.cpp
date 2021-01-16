#include "./build.hpp"

#include <Btk/thirdparty/SDL2_gfxPrimitives.h>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <SDL2/SDL.h>

#define UNPACK_COLOR(C) C.r,C.g,C.b,C.a

namespace SDL{
    //Function forward
    inline SDL_Texture *Texture(BtkTexture *texture){
        return reinterpret_cast<SDL_Texture*>(texture);
    }
    inline SDL_Renderer *Renderer(BtkRenderer *renderer){
        return reinterpret_cast<SDL_Renderer*>(renderer);
    }


    static BtkRenderer *CreateRenderer(SDL_Window *win,int index,Uint32 flags){
        return reinterpret_cast<BtkRenderer*>(SDL_CreateRenderer(win,index,flags));        
    }
    static BtkTexture  *CreateTexture(BtkRenderer *render,Uint32 fmt,int access,int w,int h){
        return reinterpret_cast<BtkTexture*>(SDL_CreateTexture(Renderer(render),fmt,access,w,h));
    }
    static void DestroyRenderer(BtkRenderer *renderer){
        SDL_DestroyRenderer(Renderer(renderer));
    }
    static void DestroyTexture(BtkTexture *texture){
        SDL_DestroyTexture(Texture(texture));
    }
    static void RenderPresent(BtkRenderer *renderer){
        SDL_RenderPresent(Renderer(renderer));
    }

    static int RenderDrawRect(BtkRenderer *render,const BtkRect *rect){
        return SDL_RenderDrawRect(Renderer(render),rect);
    }
    static int RenderDrawRects(BtkRenderer *render,const BtkRect *rect,int n){
        return SDL_RenderDrawRects(Renderer(render),rect,n);
    }
    static int RenderDrawBox(BtkRenderer *render,const BtkRect *rect){
        return SDL_RenderFillRect(Renderer(render),rect);
    }
    static int RenderDrawBoxs(BtkRenderer *render,const BtkRect *rect,int n){
        return SDL_RenderFillRects(Renderer(render),rect,n);
    }
    static int RenderDrawLine(BtkRenderer *render,int x1,int y1,int x2,int y2){
        return SDL_RenderDrawLine(Renderer(render),x1,y1,x2,y2);
    }
    static int RenderDrawLines(BtkRenderer *render,const BtkVec2 *points,int n){
        return SDL_RenderDrawLines(Renderer(render),points,n);
    }

    static int RenderDrawPoint(BtkRenderer *render,int x1,int y1){
        return SDL_RenderDrawPoint(Renderer(render),x1,y1);
    }
    static int RenderDrawPoints(BtkRenderer *render,const BtkVec2 *points,int n){
        return SDL_RenderDrawPoints(Renderer(render),points,n);
    }

    static int RenderSetDrawColor(BtkRenderer *render,Uint8 r,Uint8 g,Uint8 b,Uint8 a){
        int ret = 0;
        if(a == 255){
            ret |= SDL_SetRenderDrawBlendMode(Renderer(render),SDL_BLENDMODE_NONE);
        }
        else{
            ret |= SDL_SetRenderDrawBlendMode(Renderer(render),SDL_BLENDMODE_BLEND);
        }
        ret |= SDL_SetRenderDrawColor(Renderer(render),r,g,b,a);
        return ret;
    }
    static int RenderGetDrawColor(BtkRenderer *render,Uint8 *r,Uint8 *g,Uint8 *b,Uint8 *a){
        return SDL_GetRenderDrawColor(Renderer(render),r,g,b,a);
    }
    static void RenderGetClipRect(BtkRenderer *render,BtkRect *cliprect){
        return SDL_RenderGetClipRect(Renderer(render),cliprect);
    }
    static int  RenderSetClipRect(BtkRenderer *render,const BtkRect *cliprect){
        const SDL_Rect *r = nullptr;
        if(cliprect != nullptr){
            if(not cliprect->empty()){
                r = cliprect;
            }
        }
        return SDL_RenderSetClipRect(Renderer(render),r);
    }
    static int RenderCopy(BtkRenderer *render,BtkTexture *t,const BtkRect *src,const BtkRect *dst){
        return SDL_RenderCopy(Renderer(render),Texture(t),src,dst);
    }
    static int RenderClear(BtkRenderer *render){
        return SDL_RenderClear(Renderer(render));
    }
    static int UpdateTexture(BtkTexture *texture,const BtkRect *rect,const void *pixels,int pitch){
        return SDL_UpdateTexture(Texture(texture),rect,pixels,pitch);
    }
    static int QueryTexture(BtkTexture *texture,Uint32 *fmt,int *access,int *w,int *h){
        return SDL_QueryTexture(Texture(texture),fmt,access,w,h);
    }
    static int LockTexture(BtkTexture *texture,const BtkRect *rect,void **pixels,int *pitch){
        return SDL_LockTexture(Texture(texture),rect,pixels,pitch);
    }
    static void UnlockTexture(BtkTexture *texture){
        return SDL_UnlockTexture(Texture(texture));
    }
    static BtkTexture *CreateTextureFrom(BtkRenderer *render,SDL_Surface *surf){
        return reinterpret_cast<BtkTexture*>(SDL_CreateTextureFromSurface(Renderer(render),surf));
    }
}








extern "C"{
    BTKAPI BtkTable btk_rtbl = {
        .CreateRenderer = SDL::CreateRenderer,
        .CreateTexture  = SDL::CreateTexture,
        .CreateTextureFrom = SDL::CreateTextureFrom,

        .DestroyTexture = SDL::DestroyTexture,
        .DestroyRenderer = SDL::DestroyRenderer,

        .RenderPresent = SDL::RenderPresent,
        .RenderDrawRect = SDL::RenderDrawRect,
        .RenderDrawRects = SDL::RenderDrawRects,
        .RenderDrawBox = SDL::RenderDrawBox,
        .RenderDrawBoxs = SDL::RenderDrawBoxs,
        .RenderDrawLine = SDL::RenderDrawLine,
        .RenderDrawLines = SDL::RenderDrawLines,
        .RenderDrawPoint = SDL::RenderDrawPoint,
        .RenderDrawPoints = SDL::RenderDrawPoints,

        .RenderSetDrawColor = SDL::RenderSetDrawColor,
        .RenderGetDrawColor = SDL::RenderGetDrawColor,

        .RenderGetClipRect = SDL::RenderGetClipRect,
        .RenderSetClipRect = SDL::RenderSetClipRect,

        .RenderCopy = SDL::RenderCopy,
        .RenderClear = SDL::RenderClear,

        .QueryTexture = SDL::QueryTexture,
        .UpdateTexture = SDL::UpdateTexture,

        .LockTexture = SDL::LockTexture,
        .UnlockTexture = SDL::UnlockTexture,

        .SetError = SDL_SetError,
        .GetError = SDL_GetError,
    };
    void Btk_ResetRITable(){
        btk_rtbl.CreateRenderer = SDL::CreateRenderer;
        btk_rtbl.CreateTexture  = SDL::CreateTexture;
        btk_rtbl.CreateTextureFrom = SDL::CreateTextureFrom;

        btk_rtbl.DestroyTexture = SDL::DestroyTexture;
        btk_rtbl.DestroyRenderer = SDL::DestroyRenderer;

        btk_rtbl.RenderPresent = SDL::RenderPresent;
        btk_rtbl.RenderDrawRect = SDL::RenderDrawRect;
        btk_rtbl.RenderDrawRects = SDL::RenderDrawRects;
        btk_rtbl.RenderDrawBox = SDL::RenderDrawBox;
        btk_rtbl.RenderDrawBoxs = SDL::RenderDrawBoxs;
        btk_rtbl.RenderDrawLine = SDL::RenderDrawLine;
        btk_rtbl.RenderDrawLines = SDL::RenderDrawLines;
        btk_rtbl.RenderDrawPoint = SDL::RenderDrawPoint;
        btk_rtbl.RenderDrawPoints = SDL::RenderDrawPoints;

        btk_rtbl.RenderSetDrawColor = SDL::RenderSetDrawColor;
        btk_rtbl.RenderGetDrawColor = SDL::RenderGetDrawColor;

        btk_rtbl.RenderGetClipRect = SDL::RenderGetClipRect;
        btk_rtbl.RenderSetClipRect = SDL::RenderSetClipRect;

        btk_rtbl.RenderCopy = SDL::RenderCopy;
        btk_rtbl.RenderClear = SDL::RenderClear;

        btk_rtbl.QueryTexture = SDL::QueryTexture;
        btk_rtbl.UpdateTexture = SDL::UpdateTexture;

        btk_rtbl.LockTexture = SDL::LockTexture;
        btk_rtbl.UnlockTexture = SDL::UnlockTexture;

        btk_rtbl.SetError = SDL_SetError;
        btk_rtbl.GetError = SDL_GetError;
    }
}
namespace Btk{
    Renderer::Renderer(SDL_Window *win){
        render = Btk_CreateRenderer(win,-1,SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
        //fail to create window
        if(render == nullptr){
            throwRuntimeError(Btk_RIGetError());
        }
    }
    Texture Renderer::create_from(const PixBuf &pixbuf){
        BtkTexture *texture = Btk_CreateTextureFrom(render,pixbuf.get());
        if(texture == nullptr){
            throwRendererError(Btk_RIGetError());
        }
        return texture;
    }
    Texture Renderer::create(Uint32 fmt,int access,int w,int h){
        BtkTexture *texture = Btk_CreateTexture(render,fmt,access,w,h);
        if(texture == nullptr){
            throwRendererError(Btk_RIGetError());
        }
        return texture;
    }
    int Renderer::line(int x1,int y1,int x2,int y2,Color c){
        return lineRGBA(render,x1,y1,x2,y2,UNPACK_COLOR(c));
    }
    int Renderer::start(Color c){
        int i = 0;
        i |= Btk_RenderSetDrawColor(render,UNPACK_COLOR(c));
        i |= Btk_RenderClear(render);
        return i;
    }
    int Renderer::box(const Rect &rect,Color color){
        if(rect.empty()){
            return -1;
        }
        return boxRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                UNPACK_COLOR(color)
        );
    }
    int Renderer::rect(const Rect &rect,Color color){
        if(rect.empty()){
            return -1;
        }
        return rectangleRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                UNPACK_COLOR(color)
        );
    }

    int Renderer::rounded_box(const Rect &rect,int rad,Color color){
        if(rect.empty()){
            return -1;
        }
        return roundedBoxRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                rad,
                UNPACK_COLOR(color)
        );
    }
    int Renderer::rounded_rect(const Rect &rect,int rad,Color color){
        if(rect.empty()){
            return -1;
        }
        return roundedRectangleRGBA(
                render,
                rect.x,
                rect.y,
                rect.x + rect.w,
                rect.y + rect.h,
                rad,
                UNPACK_COLOR(color)
        );
    }
}