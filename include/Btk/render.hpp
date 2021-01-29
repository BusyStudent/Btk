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
#include <stdbool.h>
typedef SDL_Rect  BtkRect;
typedef SDL_Point BtkVec2;
#endif
struct SDL_RWops;
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
/*Here is operations code for BtkRenderer*/
/**
 * @brief Backend is OpenGL?
 * 
 * @code
 *  if(BtkRI_Control(render,BTKRI_ISOPENGL) == true){
 *      //Backend is not opengl
 *  }
 *  else{
 *      //Backend is not opengl
 *  }
 * @endcode
 * @note It will return a bool
 */
#define BTKRI_ISOPENGL 1
#define BTKRI_ISSDL    2

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
     * @brief Load a texture directly into renderer
     * 
     */
    BtkTexture  *(*LoadTextureFrom)(BtkRenderer*,SDL_RWops*);
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

    void (*RenderGetViewPort)(BtkRenderer*,BtkRect *viewport);
    int  (*RenderSetViewPort)(BtkRenderer*,const BtkRect *viewport);

    int  (*RenderCopy)(BtkRenderer*,BtkTexture *t,const BtkRect *src,const BtkRect *dst);
    int  (*RenderClear)(BtkRenderer*);

    int  (*QueryTexture)(BtkTexture*,Uint32 *fmt,int *access,int *w,int *h);
    int  (*UpdateTexture)(BtkTexture*,const BtkRect *rect,const void *pixels,int pitch);

    int  (*LockTexture)(BtkTexture*,const BtkRect *rect,void **pixels,int *pitch);
    void (*UnlockTexture)(BtkTexture *);



    /**
     * @brief Set the renderer last error
     * 
     */
    int  (*SetError)(const char *fmt,...);
    /**
     * @brief Get the renderer last error
     * 
     */
    const char *(*GetError)();
    /**
     * @brief Control the Renderer or Query Impl information
     * 
     * @param code The operation code
     * @param ... The args depended on opcode
     * @return The results based on opcode
     */
    int  (*Control)(int code,...);
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

#define Btk_RenderSetViewPort (btk_rtbl.RenderSetViewPort)
#define Btk_RenderGetViewPort (btk_rtbl.RenderGetViewPort)

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
#define Btk_LoadTexture (btk_rtbl.LoadTextureFrom)

#define Btk_RISetError (btk_rtbl.SetError)
#define Btk_RIGetError (btk_rtbl.GetError)

#define Btk_RIControl  (btk_rtbl.Control)

/*BtkRI inline functions begin*/

/**
 * @brief Check backend is opengl
 * 
 * @return true 
 * @return false 
 */
inline bool Btk_IsBackendOpenGL(){
    return btk_rtbl.Control(BTKRI_ISOPENGL);
}

/*BtkRI inline functions end*/


#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace Btk{
    class Font;
    /**
     * @brief Abstruct Renderer
     * 
     */
    class BTKAPI Renderer{
        public:
            Renderer(SDL_Window *win);
            Renderer(const Renderer &) = delete;
            ~Renderer();
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

            Rect get_viewport(){
                Rect rect;
                Btk_RenderGetViewPort(render,&rect);
                return rect;
            }
            int  set_viewport(){
                return Btk_RenderSetViewPort(render,nullptr);
            }
            int  set_viewport(const Rect &r){
                return Btk_RenderSetViewPort(render,&r);
            }


            int  copy(const Texture &t,const Rect *src,const Rect *dst){
                return Btk_RenderCopy(render,t.get(),src,dst);
            }
            int  copy(const Texture &t,const Rect &src,const Rect *dst){
                return Btk_RenderCopy(render,t.get(),&src,dst);
            }
            int  copy(const Texture &t,const Rect *src,const Rect &dst){
                return Btk_RenderCopy(render,t.get(),src,&dst);
            }
            int  copy(const Texture &t,const Rect &src,const Rect &dst){
                return Btk_RenderCopy(render,t.get(),&src,&dst);
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
            int line(const Vec2 &beg,const Vec2 &end,Color c){
                return line(beg.x,beg.y,end.x,end.y,c);
            }
            int aaline(int x1,int y1,int x2,int y2,Color c);
            
            int rect(const Rect &r,Color c);
            int box(const Rect &r,Color c);

            int pie(int x,int y,int rad,int beg,int end,Color c);
            int filled_pie(int x,int y,int rad,int beg,int end,Color c);

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
            Texture create(Uint32 fmt,TextureAccess access,int w,int h);
            /**
             * @brief Load a texture from a file
             * 
             * @param fname The filename
             * @return Texture 
             */
            Texture load(std::string_view fname);
            Texture load(RWops &);
            /**
             * @brief Copy a pixbuf into rendering target
             * 
             * @note Is is slower than texture copying
             * @param pixbuf The pixbuf
             * @param src The pixbuf area
             * @param dst The target area
             * @return int 
             */
            int copy(const PixBuf &pixbuf,const Rect *src,const Rect *dst);
            int copy(const PixBuf &pixbuf,const Rect *src,const Rect &dst){
                return copy(pixbuf,src,&dst);
            }
            int copy(const PixBuf &pixbuf,const Rect &src,const Rect *dst){
                return copy(pixbuf,&src,dst);
            }
            int copy(const PixBuf &pixbuf,const Rect &src,const Rect &dst){
                return copy(pixbuf,&src,&dst);
            }
            /**
             * @brief Draw text
             * 
             * @param x The text's x
             * @param y The text's y
             * @param c The text's color
             * @param u8 The utf8 text
             * 
             * @return 0 on success
             */
            int  text(Font &,int x,int y,Color c,std::string_view u8);
            /**
             * @brief Draw text by using c-tyle format string
             * 
             * @param x The text's x
             * @param y The text's y
             * @param c The text's color
             * @param fmt The utf8 fmt txt
             * @param ... The format args
             * @return 0 on success
             */
            int  text(Font &,int x,int y,Color c,std::string_view fmt,...);
            int  text(Font &,int x,int y,Color c,std::u16string_view u16);
            int  text(Font &,int x,int y,Color c,std::u16string_view fmt,...);
        public:
            BtkRenderer *render = nullptr;
            BtkTexture  *cache = nullptr;//< Texture cache for pixbuf copying
    };

    template<>
    struct LockGuard<BtkTexture*>{
        LockGuard(BtkTexture *t,const Rect *r = nullptr){
            texture = t;
            Btk_LockTexture(texture,r,&pixels,&pitch);
        }
        LockGuard(BtkTexture *t,const Rect &r){
            texture = t;
            Btk_LockTexture(texture,&r,&pixels,&pitch);
        }
        LockGuard(const LockGuard &) = delete;
        ~LockGuard(){
            Btk_UnlockTexture(texture);
        }
        BtkTexture*texture;
        void *     pixels;
        int        pitch;
    };
};
#endif

#endif // _BTK_RENDERER_HPP_
