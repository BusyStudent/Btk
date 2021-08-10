#if !defined(_BTK_PIXELS_HPP_)
#define _BTK_PIXELS_HPP_
#include <cstdlib>
#include <cstddef>
#include <iosfwd>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_surface.h>
#include "string.hpp"
#include "rwops.hpp"
#include "defs.hpp"
#include "rect.hpp"
struct SDL_Surface;

#define BTK_MAKE_FMT(NAME) static constexpr Uint32 NAME = \
    SDL_PIXELFORMAT_##NAME

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
    /**
     * @brief Color in opengl
     * 
     */
    struct GLColor{
        GLColor() = default;
        GLColor(const GLColor &) = default;
        GLColor(float r,float g,float b,float a = 1.0f){
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }
        GLColor(Color c){
            this->r = 1.0f / 255 * c.r;
            this->g = 1.0f / 255 * c.g;
            this->b = 1.0f / 255 * c.b;
            this->a = 1.0f / 255 * c.a;
        }
        operator Color() const noexcept{
            return {
                Uint8(r / 255),
                Uint8(g / 255),
                Uint8(b / 255),
                Uint8(a / 255),
            };
        }
        float r;
        float g;
        float b;
        float a;
    };
    struct HSVColor{
        HSVColor() = default;
        HSVColor(const HSVColor &) = default;
        HSVColor(float h,float s,float v,Uint8 a = 255){
            this->h = h;
            this->s = s;
            this->v = v;
            this->a = a;
        }
        float h;
        float s;
        float v;
        Uint8 a;//<Alpha
    };
    using HSBColor = HSVColor;
    //PixelBuffer
    class BTKAPI PixBuf{
        public:
            PixBuf(SDL_Surface *s = nullptr):surf(s){};//empty 
            PixBuf(int w,int h,Uint32 format);//Create a buffer
            //LoadFromFile
            PixBuf(u8string_view file);
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
            //FIXME:Why save png and jpg in SDL_image doesnnot work
            //It exported an black image
            void save_bmp(RWops &);
            void save_bmp(u8string_view fname);
            /**
             * @brief Save the pixel buffer
             * 
             * @param rw The output stream
             * @param type The output image format
             * @param quality The compress quality(0 - 10) default(0)
             */
            void save(RWops &,u8string_view type,int quality = 0);
            /**
             * @brief Save the pixel buffer
             * 
             * @param filename The output filename
             * @param type The output image format
             * @param quality The compress quality(0 - 10) default(0)
             */
            void save(u8string_view filename,u8string_view type = {},int quality = 0);
            
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
             * @brief Make sure the pixbuf is unique
             * 
             */
            void begin_mut();
            /**
             * @brief Convert a pixbuf's format
             * 
             * @param fmt The format
             * @return PixBuf The pixel buf
             */
            PixBuf convert(Uint32 fmt) const;
            PixBuf zoom(double w_factor,double h_factor);
            PixBuf zoom_to(int w,int h){
                return zoom(double(w) / double(this->w()),double(h) / double(this->h()));
            }
            /**
             * @brief Copy it into
             * 
             * @param buf 
             * @param src 
             * @param dst 
             */
            void bilt(const PixBuf &buf,const Rect *src,Rect *dst);
            //Some static method to load image
            static PixBuf FromFile(u8string_view file);
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

        BTK_MAKE_FMT(UNKNOWN);
        BTK_MAKE_FMT(RGB332);
        BTK_MAKE_FMT(RGB555);
        BTK_MAKE_FMT(BGR555);
        BTK_MAKE_FMT(RGB565);
        BTK_MAKE_FMT(RGBA32);

        //YUV
        BTK_MAKE_FMT(YV12);
        BTK_MAKE_FMT(IYUV);
        BTK_MAKE_FMT(YUY2);
        BTK_MAKE_FMT(YVYU);
        BTK_MAKE_FMT(UYVY);
        BTK_MAKE_FMT(NV12);
        BTK_MAKE_FMT(NV21);

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
            u8string_view name() const;
            SDL_PixelFormat *operator ->() const noexcept{
                return fmt;
            }
            SDL_PixelFormat *get() const noexcept{
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
    /**
     * @brief TextureFlags from nanovg
     * 
     */
    enum class TextureFlags:int{
        Linear           = 0,        // Image interpolation is Linear instead Nearest(default)
        GenerateMipmaps  = 1<<0,     // Generate mipmaps during creation of the image.
        RepeatX			 = 1<<1,	 // Repeat image in X direction.
        RepeatY			 = 1<<2,	 // Repeat image in Y direction.
        Flips			 = 1<<3,	 // Flips (inverses) image in Y direction when rendered.
        Premultiplied	 = 1<<4,	 // Image data has premultiplied alpha.
        Nearest			 = 1<<5,	 // Image interpolation is Nearest instead Linear
    };
    //TextureFlags operators
    // inline TextureFlags operator |(TextureFlags a,TextureFlags b){
    //     return static_cast<TextureFlags>(int(a) | int(b));
    // }
    // inline TextureFlags operator +(TextureFlags a,TextureFlags b){
    //     return static_cast<TextureFlags>(int(a) | int(b));
    // }
    BTK_FLAGS_OPERATOR(TextureFlags,int);
    //RendererTexture
    using TextureID = int;
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
                return texture <= 0;
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
            
            void update_yuv(
                const Rect *rect,
                const Uint8 *y_plane, int y_pitch,
                const Uint8 *u_plane, int u_pitch,
                const Uint8 *v_plane, int v_pitch,
                void *convert_buf = nullptr
            );

            void clear();
            /**
             * @brief Get the texture's native handler,
             *        It is based on the backend impl
             * 
             * @param p_handle The pointer to the handle
             */
            void native_handle(void *p_handle);
            template<class T>
            T    native_handle(){
                T handle;
                native_handle(reinterpret_cast<void*>(&handle));
                return handle;
            }
            /**
             * @brief Clone the texture
             * 
             * @return Texture 
             */
            Texture clone() const;
            /**
             * @brief Read the texture into pixbuffer
             * 
             * @return PixBuf 
             */
            PixBuf  dump() const;
            /**
             * @brief Get TextureFlags
             * 
             * @return TextureFlags 
             */
            TextureFlags flags() const;
            /**
             * @brief Set the flags 
             * 
             * @param flags 
             */
            void set_flags(TextureFlags flags);
        private:
            int texture = 0;//< NVG Image ID
            Renderer *render = nullptr;//Renderer
        friend struct Renderer;
    };
    /**
     * @brief Gif Decoding class
     * 
     */
    class BTKAPI GifImage{
        public:
            explicit GifImage(void *p = nullptr):pimpl(p){};
            GifImage(const GifImage &) = delete;
            GifImage(GifImage &&g){
                pimpl = g.pimpl;
                g.pimpl = nullptr;
            }
            ~GifImage();
            /**
             * @brief Get the size of the image
             * 
             * @return Size 
             */
            Size size() const;
            bool empty() const{
                return pimpl == nullptr;
            }
            /**
             * @brief How much frame do we have?
             * 
             * @return size_t 
             */
            size_t image_count() const;
            /**
             * @brief Get the PixBuf by index
             * 
             * @param index 
             * @return PixBuf 
             */
            PixBuf get_image(size_t index) const;
            /**
             * @brief Get the Image and write it info the buffer
             * 
             * @param index the image index
             * @param buf The pixel buf(buf.size() should equal to the gif.size())
             * @param delay How much ms should we delay to the next frame
             */
            void   update_frame(size_t index,PixBuf &buf,int *delay = nullptr) const;

            GifImage &operator =(GifImage &&);
            static GifImage FromRwops(RWops &);
            static GifImage FromFile(u8string_view fname);
        private:
            void *pimpl;
    };
    /**
     * @brief Decoder Interface like WIC
     * 
     */
    class ImageDecoder{
        public:
            virtual inline ~ImageDecoder(){};

            /**
             * @brief Get information of images
             * 
             * 
             * @param p_n_frame Point to the count of frames
             * @param p_size Point to size
             */
            virtual void query_info(
                size_t *p_n_frame,
                PixelFormat *p_fmt
            ) = 0;
            virtual void query_frame(
                size_t frame_index
            ) = 0;
            virtual void read_pixels(
                size_t frame_index,
                void *pixelss
            ) = 0;
            /**
             * @brief Open a stream
             * 
             * @param rwops The point to SDL_RWops
             * @param autoclose Should we close the SDL_RWops when the stream is closed
             */
            virtual void open(SDL_RWops *rwops,bool autoclose = false) = 0;
            /**
             * @brief Close the stream
             * 
             */
            virtual void close() = 0;

            void open(RWops &rwops){
                open(rwops.get());
            }

            PixelFormat image_format(){
                PixelFormat fmt;
                query_info(nullptr,&fmt);
                return fmt;
            }


            //Create by type and vendor
            static ImageDecoder *Create(u8string_view type,u8string_view vendor = {});
        private:
            SDL_RWops *fstream = nullptr;
            bool       auto_close = false;
    };
    //TODO
    class ImageEncoder{

    };
    /**
     * @brief Convert string to color
     * 
     * @param text 
     * @return BTKAPI 
     */
    BTKAPI Color ParseColor(u8string_view text);
    BTKAPI std::ostream &operator <<(std::ostream &,Color c);
};

#endif // _BTK_PIXELS_HPP_
