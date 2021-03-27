#if !defined(_BTK_PIXELS_HPP_)
#define _BTK_PIXELS_HPP_
#include <string_view>
#include <cstdlib>
#include <cstddef>
#include <iosfwd>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include "defs.hpp"
#include "rect.hpp"
struct SDL_Surface;
namespace Btk{
    class RWops;
    class Renderer;
    /**
     * @brief Color structure
     * 
     */
    struct Color:public SDL_Color{
        Color() = default;
        Color(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        Color(const SDL_Color &c):Color(c.r,c.g,c.b,c.a){}
        Color(const Color &) = default;
        Color &operator =(const Color &) = default;
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
            Size size() const noexcept{
                return {surf->w,surf->h};
            }
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
            size_t pixels_size() const noexcept{
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
            void lock() const;
            void unlock() const noexcept;
            //Set RLE
            void set_rle(bool val = true);
            /**
             * @brief Convert a pixbuf's format
             * 
             * @param fmt The format
             * @return PixBuf The pixel buf
             */
            PixBuf convert(Uint32 fmt) const;
            //Some static method to load image
            static PixBuf FromFile(std::string_view file);
            static PixBuf FromFile(FILE *f);
            static PixBuf FromMem(const void *mem,size_t size);
            static PixBuf FromRWops(RWops &);
            static PixBuf FromXPMArray(const char *const*);
        private:
            SDL_Surface *surf;
    };
    /**
     * @brief Pixels format
     * 
     */
    struct PixelFormat{
        PixelFormat() = default;
        PixelFormat(Uint32 val):fmt(val){}
        
        operator Uint32() const noexcept{
            return fmt;
        }
        Uint32 fmt;
    };
    /**
     * @brief Pixels format detail
     * 
     */
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
    /**
     * @brief TextureAccess(same def in SDL_render.h)
     * 
     */
    enum class TextureAccess:int{
        Static,
        Streaming,
        Target
    };
    //RendererTexture
    class BTKAPI Texture{
        public:
            /**
             * @brief Texture's information
             * 
             */
            struct Information{
                TextureAccess access;//< Texture access
                PixelFormat   format;//< Pixels format
                int w;
                int h;
            };
        public:
            /**
             * @brief Construct a new Texture object
             * 
             * @param id 
             * @param r 
             */
            Texture(int id,Renderer *r):texture(id),render(r){}
            /**
             * @brief Construct a new empty Texture object
             * 
             */
            Texture():texture(0),render(nullptr){};
            Texture(const Texture &) = delete;
            Texture(Texture &&t){
                texture = t.texture;
                render = t.render;

                t.texture = 0;
                t.render = nullptr;
            }
            ~Texture();
            /**
             * @brief Get the size(w and h)
             * 
             * @return W and H
             */
            Size size() const;
            int w() const{
                return size().w;
            }
            int h() const{
                return size().h;
            }
            //check is empty
            bool empty() const noexcept{
                return texture == 0;
            }
            //assign
            //Texture &operator =(BtkTexture*);
            Texture &operator =(Texture &&);
            /**
             * @brief clear the texture
             * 
             * @return Texture& 
             */
            Texture &operator =(std::nullptr_t){
                clear();
                return *this;
            }
            int get() const noexcept{
                return texture;
            }
            int detach() noexcept{
                int i = texture;
                texture = 0;
                render = nullptr;
                return i;
            }
            #if 0
            /**
             * @brief Update a texture's pixels
             * @note This is a very slow operation
             * 
             * @param r The area you want to update(nullptr to all texture)
             * @param pixels The pixels pointer
             * @param pitch The pixels pitch
             */
            void update(const Rect *r,void *pixels,int pitch);
            /**
             * @brief Lock a Streaming Texture
             * 
             * @param rect The area you want to lock(nullptr to all texture)
             * @param pixels The pixels pointer's pointer
             * @param pitch The texture's pixels pitch pointer
             */
            void lock(const Rect *rect,void **pixels,int *pitch);

            void lock(const Rect &rect,void **pixels,int *pitch){
                lock(&rect,pixels,pitch);
            }
            void lock(void **pixels,int *pitch){
                lock(nullptr,pixels,pitch);
            }
            void update(const Rect &r,void *pixels,int pitch){
                update(&r,pixels,pitch);
            }
            void update(void *pixels,int pitch){
                update(nullptr,pixels,pitch);
            }
            /**
             * @brief Unlock the texture
             * 
             */
            void unlock();
            /**
             * @brief Get the texture's information
             * 
             * @return Information 
             */
            Information information() const;
            #endif
            /**
             * @brief Update whole texture
             * 
             * @param pixels The RGBA32 formated pixels(nullptr on no-op)
             */
            void update(const void *pixels);
            /**
             * @brief Update texture
             * 
             * @param rect The area you want to update
             * @param pixels The RGBA32 formated pixels(nullptr on no-op)
             */
            void update(const Rect &rect,const void *pixels);
            /**
             * @brief Update texture by pixbuf
             * 
             * @note The pixbuf's size() must be equal to the texture
             * @param pixbuf 
             */
            void update(const PixBuf &pixbuf);
            void clear();
            /**
             * @brief Get the texture's native handler,
             *        It is based on the backend impl
             * 
             * @param p_handle The pointer to the handle
             */
            void native_handle(void *p_handle);
        private:
            int texture = 0;//< NVG Image ID
            Renderer *render = nullptr;//Renderer
        friend struct Renderer;
    };
    /**
     * @brief Gif Decoding class
     * 
     */
    class BTKAPI Gif{
        public:
            Gif():pimpl(nullptr){};
            ~Gif();
            static Gif FromRwops(RWops &);
        private:
            void *pimpl;
    };
};

#endif // _BTK_PIXELS_HPP_
