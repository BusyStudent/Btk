#if !defined(_BTK_PIXELS_HPP_)
#define _BTK_PIXELS_HPP_
#include <string_view>
#include <cstdlib>
#include <cstddef>
#include <iosfwd>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include "defs.hpp"
struct SDL_Surface;
struct SDL_Texture;
namespace Btk{
    class RWops;
    //Color struct
    struct Color:public SDL_Color{

    };
    //PixelBuffer
    class BTKAPI PixBuf{
        public:
            PixBuf(SDL_Surface *s = nullptr):surf(s){};//empty 
            PixBuf(int w,int h,Uint32 format);//Create a buffer
            //LoadFromFile
            PixBuf(std::string_view file);
            //Move construct
            PixBuf(const PixBuf &) = delete;
            PixBuf(PixBuf && s){
                surf = s.surf;
                s.surf = nullptr;
            };
            ~PixBuf();
            //Get a copy of this PixBuf
            PixBuf clone() const;
            //Get a ref of this PixBuf
            PixBuf ref() const;
            //operators

            //save PixBuf
            void save_png(RWops &,int quality);
            void save_jpg(RWops &,int quality);
            void save_bmp(RWops &);

            void save_png(std::string_view fname,int quality);
            void save_jpg(std::string_view fname,int quality);
            void save_bmp(std::string_view fname);
            
            PixBuf &operator =(SDL_Surface *);//assign
            PixBuf &operator =(PixBuf &&);//assign
            //check empty
            bool empty() const noexcept{
                return surf == nullptr;
            }
            //Get informations
            int w() const noexcept{
                return surf->w;
            }
            int h() const noexcept{
                return surf->h;
            }
            int refcount() const noexcept{
                return surf->refcount;
            }
            int pitch() const noexcept{
                return surf->pitch;
            }
            //return its pixels size
            size_t size() const noexcept{
                return pitch() * h();
            }
            //must lock it before access its pixels
            bool must_lock() const noexcept{
                return SDL_MUSTLOCK(surf);
            }
            //get pixels
            template<class T = void>
            T *pixels() const noexcept{
                return static_cast<T*>(surf->pixels);
            }
            SDL_Surface *get() const noexcept{
                return surf;
            }
            SDL_Surface *operator ->() const noexcept{
                return surf;
            }
            //Lock and Unlock
            void lock();
            void unlock() noexcept;
            //Set RLE
            void set_rle(bool val = true);
            //Some static method to load image
            static PixBuf FromFile(std::string_view file);
            static PixBuf FromFile(FILE *f);
            static PixBuf FromMem(const void *mem,size_t size);
            static PixBuf FromRWops(RWops &);
            static PixBuf FromXPMArray(const char *const*);
        private:
            SDL_Surface *surf;
    };
    //PixelFormat
    class BTKAPI PixFmt{
        public:
            //Create a pixel format
            PixFmt(Uint32 pix_fmt);
            PixFmt(const PixFmt &pixfmt):
                PixFmt(pixfmt.fmt->format){};
            PixFmt(PixFmt &&f){
                fmt = f.fmt;
                f.fmt = nullptr;
            }
            ~PixFmt();

            //Map RPG
            Uint32 map_rgb (Uint8 r,Uint8 g,Uint8 b) const;
            Uint32 map_rgba(Uint8 r,Uint8 g,Uint8 b,Uint8 a) const;

            Uint32 map_rgb(Color color) const{
                return map_rgb(
                    color.r,
                    color.g,
                    color.b
                );
            };
            Uint32 map_rgba(Color color) const{
                return map_rgba(
                    color.r,
                    color.g,
                    color.b,
                    color.a
                );
            };
            //Get its name
            std::string_view name() const;
            SDL_PixelFormat *operator ->() const noexcept{
                return fmt;
            }
            operator Uint32() const noexcept{
                return fmt->format;
            };
        private:
            SDL_PixelFormat *fmt;
    };
    //RendererTexture
    class BTKAPI Texture{
        public:
            Texture(SDL_Texture *t = nullptr):texture(t){};
            Texture(const Texture &) = delete;
            Texture(Texture &&t){
                texture = t.texture;
                t.texture = nullptr;
            }
            ~Texture();
            int w() const;
            int h() const;
            //check is empty
            bool empty() const noexcept{
                return texture == nullptr;
            }
            //assign
            Texture &operator =(SDL_Texture*);
            Texture &operator =(Texture &&);
            //Get pointer
            SDL_Texture *get() const noexcept{
                return texture;
            }
        private:
            SDL_Texture *texture;
        friend struct Renderer;
    };
    typedef PixFmt PixelFormat;
    typedef PixFmt PixFormat;
};

#endif // _BTK_PIXELS_HPP_
