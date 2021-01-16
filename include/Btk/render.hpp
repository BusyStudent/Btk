#if !defined(_BTK_RENDERER_HPP_)
#define _BTK_RENDERER_HPP_
#include "defs.hpp"
struct SDL_Window;

#ifdef __cplusplus
#include "pixels.hpp"
#include "rect.hpp"
//Basic type defs
typedef Btk::Rect BtkRect;
typedef Btk::Vec2 BtkVec2;
extern "C"{

#else
#include <SDL2/SDL_rect.h>
typedef SDL_Rect  BtkRect;
typedef SDL_Point BtkVec2;
#endif
/**
 * @brief Abstruct Texture,prepared for different backend
 * 
 */
typedef struct BtkTexture  BtkTexture;
/**
 * @brief Abstruct Renderer,prepared for different backend
 * 
 */
typedef struct BtkRenderer BtkRenderer;
/**
 * @brief Renderer Interface Table
 * 
 */
struct BtkTable{
    /**
     * @brief Create a Renderer 
     * 
     * @param win The SDL_Window
     * @param index The driver index (default -1)
     * @param flags The renderer flags
     * @return BtkRenderer* 
     */
    BtkRenderer *(*CreateRenderer)(SDL_Window *win,int index,Uint32 flags);
    /**
     * @brief Create a texture in a Renderer
     * 
     */
    BtkTexture  *(*CreateTexture)(BtkRenderer*,Uint32 fmt,int access,int w,int h);
    BtkTexture  *(*CreateTextureFrom)(BtkRenderer*,SDL_Surface*);
    /**
     * @brief Destroy a texture
     * 
     */
    void (*DestroyTexture)(BtkTexture *);
    void (*DestroyRenderer)(BtkRenderer *);

    void (*RenderPresent)(BtkRenderer *);

    int  (*RenderDrawRect) (BtkRenderer*,const BtkRect *);
    int  (*RenderDrawRects)(BtkRenderer*,const BtkRect *,int n);
    int  (*RenderDrawBox )(BtkRenderer*,const BtkRect *);
    int  (*RenderDrawBoxs)(BtkRenderer*,const BtkRect *,int n);
    int  (*RenderDrawLine)(BtkRenderer*,int x1,int y1,int x2,int y2);
    int  (*RenderDrawLines)(BtkRenderer*,const BtkVec2 *vec2s,int n);
    int  (*RenderDrawPoint)(BtkRenderer*,int x1,int y1);
    int  (*RenderDrawPoints)(BtkRenderer*,const BtkVec2 *points,int n);

    int  (*RenderSetDrawColor)(BtkRenderer*,Uint8 r,Uint8 g,Uint8 b,Uint8 a);
    int  (*RenderGetDrawColor)(BtkRenderer*,Uint8 *r,Uint8 *g,Uint8 *b,Uint8 *a);

    void (*RenderGetClipRect)(BtkRenderer*,BtkRect *cliprect);
    int  (*RenderSetClipRect)(BtkRenderer*,const BtkRect *cliprect);

    int  (*RenderCopy)(BtkRenderer*,BtkTexture *t,const BtkRect *src,const BtkRect *dst);
    int  (*RenderClear)(BtkRenderer*);

    int  (*QueryTexture)(BtkTexture*,Uint32 *fmt,int *access,int *w,int *h);
    int  (*UpdateTexture)(BtkTexture*,const BtkRect *rect,const void *pixels,int pitch);

    int  (*LockTexture)(BtkTexture*,const BtkRect *rect,void **pixels,int *pitch);
    void (*UnlockTexture)(BtkTexture *);

    int  (*SetError)(const char *fmt,...);
    const char *(*GetError)();
};
extern BTKAPI struct BtkTable btk_rtbl;

BTKAPI void Btk_ResetRITable();
#define Btk_CreateRenderer (btk_rtbl.CreateRenderer)
#define Btk_CreateTexture (btk_rtbl.CreateTexture)
#define Btk_DestroyTexture (btk_rtbl.DestroyTexture)
#define Btk_DestroyRenderer (btk_rtbl.DestroyRenderer)

#define Btk_RenderDrawRect  (btk_rtbl.RenderDrawRect)
#define Btk_RenderDrawRects (btk_rtbl.RenderDrawRects)

#define Btk_RenderDrawBox  (btk_rtbl.RenderDrawBox)
#define Btk_RenderDrawBoxs (btk_rtbl.RenderDrawBoxs)

#define Btk_RenderDrawLine  (btk_rtbl.RenderDrawLine)
#define Btk_RenderDrawLines (btk_rtbl.RenderDrawLines)

#define Btk_RenderDrawPoint  (btk_rtbl.RenderDrawPoint)
#define Btk_RenderDrawPoints (btk_rtbl.RenderDrawPoints)

#define Btk_RenderSetClipRect (btk_rtbl.RenderSetClipRect)
#define Btk_RenderGetClipRect (btk_rtbl.RenderGetClipRect)

#define Btk_RenderSetDrawColor (btk_rtbl.RenderSetDrawColor)

#define Btk_RenderCopy    (btk_rtbl.RenderCopy)
#define Btk_RenderClear    (btk_rtbl.RenderClear)
#define Btk_RenderPresent (btk_rtbl.RenderPresent)


#define Btk_QueryTexture (btk_rtbl.QueryTexture)
#define Btk_UpdateTexture (btk_rtbl.UpdateTexture)

#define Btk_LockTexture (btk_rtbl.LockTexture)
#define Btk_UnlockTexture (btk_rtbl.UnlockTexture)

#define Btk_CreateTextureFromSurface (btk_rtbl.CreateTextureFrom)
#define Btk_CreateTextureFrom (btk_rtbl.CreateTextureFrom)

#define Btk_RISetError (btk_rtbl.SetError)
#define Btk_RIGetError (btk_rtbl.GetError)
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace Btk{
    /**
     * @brief Abstruct Renderer
     * 
     */
    class BTKAPI Renderer{
        public:
            Renderer(SDL_Window *win);
            Renderer(const Renderer &) = delete;
            ~Renderer(){
                Btk_DestroyRenderer(render);
            }
            Rect get_cliprect(){
                Rect rect;
                Btk_RenderGetClipRect(render,&rect);
                return rect;
            }
            int  set_cliprect(){
                return Btk_RenderSetClipRect(render,nullptr);
            }
            int  set_cliprect(const Rect &r){
                return Btk_RenderSetClipRect(render,&r);
            }

            int  copy(const Texture &t,const Rect *src,const Rect *dst){
                return Btk_RenderCopy(render,t.get(),src,dst);
            }
            int clear(){
                return Btk_RenderClear(render);
            }
            void done(){
                Btk_RenderPresent(render);
            }
            /**
             * @brief Destroy the renderer
             * 
             */
            void destroy(){
                Btk_DestroyRenderer(render);
                render = nullptr;
            }
            /**
             * @brief Start drawing
             * 
             */
            int start(Color c);
            int line(int x1,int y1,int x2,int y2,Color c);
            int aaline(int x1,int y1,int x2,int y2,Color c);
            
            int rect(const Rect &r,Color c);
            int box(const Rect &r,Color c);

            int rounded_box(const Rect &r,int rad,Color c);
            int rounded_rect(const Rect &r,int rad,Color c);
            /**
             * @brief Create a Texture from Pixbuf
             * 
             * @param pixbuf The Pixbuffer
             * @return Texture The texture
             */
            Texture create_from(const PixBuf &pixbuf);
            /**
             * @brief Create a Texture
             * 
             * @param fmt 
             * @param access 
             * @param w 
             * @param h 
             * @return Texture 
             */
            Texture create(Uint32 fmt,int access,int w,int h);
        public:
            BtkRenderer *render;
    };
};
#endif

#endif // _BTK_RENDERER_HPP_
