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
    class PixBuf;
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

        bool transparent() const noexcept{
            return a == 0;
        }

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
        /**
         * @brief Construct A HSVColor from color
         * 
         */
        HSVColor(Color c){
            float r = c.r / 255.0f;
            float g = c.g / 255.0f;
            float b = c.b / 255.0f;
            float max = Btk::max(r,Btk::max(g,b));
            float min = Btk::min(r,Btk::min(g,b));
            float delta = max - min;
            if(max == min){
                h = 0;
            }
            else if(max == r){
                h = 60 * (g - b) / delta;
            }
            else if(max == g){
                h = 60 * (b - r) / delta + 120;
            }
            else if(max == b){
                h = 60 * (r - g) / delta + 240;
            }
            if(max == 0){
                s = 0;
            }
            else{
                s = delta / max;
            }
            v = max;
            a = c.a;
        }
        float h;
        float s;
        float v;
        Uint8 a;//<Alpha
    };
    using HSBColor = HSVColor;
    //PixelBuffer
    class BTKAPI PixBufRef{
        public:
            PixBufRef(SDL_Surface *s = nullptr):surf(s){};//empty 
            //Move construct
            PixBufRef(const PixBufRef &) = default;
            PixBufRef(PixBufRef && s){
                surf = s.surf;
                s.surf = nullptr;
            };
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
            PixBuf resize(int w,int h) const;
            PixBuf blur(float sigma,int radius = 0) const;
            PixBuf zoom(double w_factor,double h_factor) const;
            /**
             * @brief Copy a area into a new buf
             * 
             * @param x 
             * @param y 
             * @param w 
             * @param h 
             * @return PixBuf 
             */
            PixBuf copy(int x,int y,int w,int h) const;
            // PixBuf zoom_to(int w,int h){
            //     return zoom(double(w) / double(this->w()),double(h) / double(this->h()));
            // }
            /**
             * @brief Copy it into
             * 
             * @param buf 
             * @param src 
             * @param dst 
             */
            void bilt(PixBufRef buf,const Rect *src,Rect *dst);
        protected:
            SDL_Surface *surf;
        friend class PixBuf;
    };
    class BTKAPI PixBuf:public PixBufRef{
        public:
            using PixBufRef::PixBufRef;

            PixBuf(int w,int h,Uint32 format);//Create a buffer
            //Simple take ref
            PixBuf(const PixBuf &buf){
                surf = buf.surf;
                if(surf != nullptr){
                    ++(surf->refcount);
                }
            }
            /**
             * @brief Construct a new Pix Buf object
             * 
             * @param buf 
             */
            PixBuf(PixBufRef buf){
                surf = buf.surf;
                if(surf != nullptr){
                    ++(surf->refcount);
                }
            }
            //LoadFromFile
            PixBuf(u8string_view file);
            ~PixBuf();
            
            //Get a copy of this PixBuf
            PixBuf clone() const;
            //Get a ref of this PixBuf
            PixBuf ref() const;
            SDL_Surface *detach() {
                auto s = surf;
                surf = nullptr;
                return s;
            }
            /**
             * @brief Make sure the pixbuf is unique
             * 
             */
            void begin_mut();


            PixBuf &operator =(SDL_Surface *);//assign
            PixBuf &operator =(PixBuf &&);//assign

            //Some static method to load image
            static PixBuf FromFile(u8string_view file);
            static PixBuf FromFile(FILE *f);
            static PixBuf FromMem(const void *mem,size_t size);
            static PixBuf FromRWops(RWops &);
            static PixBuf FromXPMArray(const char *const*);
        private:
            //Utils
            static void _Delete(void *);
            static void _Ref(void *);
    };
    inline PixBuf PixBufRef::zoom(double w_factor,double h_factor) const{
        return resize(surf->w * w_factor,surf->h * h_factor);
    }
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
        bool has_alpha() const noexcept{
            return SDL_ISPIXELFORMAT_ALPHA(fmt);
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
    BTK_FLAGS_OPERATOR(TextureFlags,int);
    //RendererTexture
    using TextureID = int;
    class Texture;
    /**
     * @brief Texture's reference
     * 
     */
    class BTKAPI TextureRef{
        public:
            /**
             * @brief Construct a new Texture object
             * 
             * @param id 
             * @param r 
             */
            TextureRef(int id,Renderer *r):texture(id),render(r){}
            /**
             * @brief Construct a new empty Texture object
             * 
             */
            TextureRef() = default;
            TextureRef(const TextureRef &) = default;
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
                return texture < 0;
            }

            TextureID get() const noexcept{
                return texture;
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
            void update(PixBufRef pixbuf);
            /**
             * @brief Update the texture by format
             * 
             * @param rect 
             * @param pixels 
             * @param fmt 
             */
            void update(const Rect *rect,const void *pixels,Uint32 fmt);
            
            void update_yuv(
                const Rect *rect,
                const Uint8 *y_plane, int y_pitch,
                const Uint8 *u_plane, int u_pitch,
                const Uint8 *v_plane, int v_pitch,
                void *convert_buf = nullptr
            );
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
        protected:
            TextureID texture = -1;//< NVG Image ID
            Renderer *render = nullptr;//Renderer
        friend struct Renderer;
    };
    class BTKAPI Texture:public TextureRef{
        public:
            using TextureRef::TextureRef;
            Texture(const Texture &) = delete;
            Texture(Texture &&t){
                texture = t.texture;
                render = t.render;

                t.texture = -1;
                t.render = nullptr;
            }

            ~Texture(){
                clear();
            }


            TextureID detach() noexcept{
                TextureID i = texture;
                texture = -1;
                render = nullptr;
                return i;
            }
            /**
             * @brief Release the texture
             * 
             */
            void clear();

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
    class BTKAPI ImageDecoder{
        public:
            ImageDecoder() = default;
            ImageDecoder(const ImageDecoder &) = delete;
            virtual ~ImageDecoder();

            /**
             * @brief Get information of images
             * 
             * 
             * @param p_n_frame Point to the count of frames
             * @param p_fmt Point to format
             */
            virtual void query_info(
                size_t *p_n_frame,
                PixelFormat *p_fmt
            ) = 0;
            /**
             * @brief Query the frame information
             * 
             * @param frame_index 
             * @param p_size The frame size
             * @param p_delay The pointer to delay(in gif)
             */
            virtual void query_frame(
                size_t frame_index,
                Size *p_size,
                int *p_delay = nullptr
            ) = 0;
            /**
             * @brief Read frame
             * 
             * @param frame_index 
             * @param rect 
             * @param pixels The pointer to pixels
             * @param wanted The pointer to the format we wanted(or nullptr)
             */
            virtual void read_pixels(
                size_t frame_index,
                const Rect *rect,
                void *pixels,
                const PixelFormat *wanted = nullptr
            ) = 0;
            /**
             * @brief Open a stream
             * 
             * @param rwops The point to SDL_RWops
             * @param autoclose Should we close the SDL_RWops when the stream is closed
             */
            void open(SDL_RWops *rwops,bool autoclose = false);
            void open(u8string_view filename);
            void open(RWops &rwops){
                open(rwops.get());
            }
            void open(RWops &&rwops){
                open(rwops.get(),true);
                //Succeed
                rwops.detach();
            }
            /**
             * @brief Close the stream
             * 
             */
            void close();
            bool is_opened() const noexcept{
                return _is_opened;
            }

            /**
             * @brief Get PixelFormat of it
             * 
             * @return PixelFormat 
             */
            PixelFormat container_format(){
                PixelFormat fmt;
                query_info(nullptr,&fmt);
                return fmt;
            }
            size_t frame_count(){
                size_t n;
                query_info(&n,nullptr);
                return n;
            }
            Size frame_size(size_t idx){
                Size s;
                query_frame(idx,&s);
                return s;
            }
            int  frame_delay(size_t idx){
                int delay;
                query_frame(idx,nullptr,&delay);
                return delay;
            }
            PixBuf read_frame(size_t frame_idx,const Rect *r = nullptr);
        protected:
            virtual void decoder_open() = 0;
            virtual void decoder_close() = 0;

            SDL_RWops *stream() const noexcept{
                return fstream;
            }

        private:
            SDL_RWops *fstream = nullptr;
            bool       auto_close = false;
            bool       _is_opened = false;
    };
    //TODO
    class ImageEncoder{

    };
    //Create by type and vendor
    BTKAPI ImageDecoder *CreateImageDecoder(u8string_view type,u8string_view vendor = {});
    BTKAPI ImageEncoder *CreateImageEncoder(u8string_view type,u8string_view vendor = {});
    /**
     * @brief Create a Image Decoder object
     * 
     * @param rwops The current stream
     * @param autoclose 
     * @return ImageDecoder* 
     */
    BTKAPI ImageDecoder *CreateImageDecoder(SDL_RWops *rwops,bool autoclose = false);
    /**
     * @brief Create a Image Decoder object
     * 
     * @param rwops The current stream
     * @return ImageDecoder* 
     */
    inline ImageDecoder *CreateImageDecoder(RWops &rwops){
        return CreateImageDecoder(rwops.get(),false);
    }
    /**
     * @brief Convert string to color
     * 
     * @param text 
     * @return BTKAPI 
     */
    BTKAPI Color ParseColor(u8string_view text);
    BTKAPI std::ostream &operator <<(std::ostream &,Color c);


    //MAP / GET COLOR
    inline Uint32 MapRGBA32(Color c){
        Uint32 pixel;
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        //Big endian RGBA8888
        reinterpret_cast<Uint8*>(&pixel)[0] = c.a;
        reinterpret_cast<Uint8*>(&pixel)[1] = c.b;
        reinterpret_cast<Uint8*>(&pixel)[2] = c.g;
        reinterpret_cast<Uint8*>(&pixel)[3] = c.r;
        #else
        //little endian ABGR8888
        reinterpret_cast<Uint8*>(&pixel)[0] = c.r;
        reinterpret_cast<Uint8*>(&pixel)[1] = c.g;
        reinterpret_cast<Uint8*>(&pixel)[2] = c.b;
        reinterpret_cast<Uint8*>(&pixel)[3] = c.a;
        #endif
        return pixel;

    }
    inline Uint32 MapRGBA32(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
        return MapRGBA32({r,g,b,a});
    }

    inline Color GetRGBA32(Uint32 pixel){
        Color c;
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        //Big endian RGBA8888
        c.a = reinterpret_cast<Uint8*>(&pixel)[0];
        c.b = reinterpret_cast<Uint8*>(&pixel)[1];
        c.g = reinterpret_cast<Uint8*>(&pixel)[2];
        c.r = reinterpret_cast<Uint8*>(&pixel)[3];
        #else
        //little endian ABGR8888
        c.r = reinterpret_cast<Uint8*>(&pixel)[0];
        c.g = reinterpret_cast<Uint8*>(&pixel)[1];
        c.b = reinterpret_cast<Uint8*>(&pixel)[2];
        c.a = reinterpret_cast<Uint8*>(&pixel)[3];
        #endif
        return c;
    }
    inline GLColor GetRGBA32f(Uint32 pixel){
        return GetRGBA32(pixel);
    }

};

#endif // _BTK_PIXELS_HPP_
