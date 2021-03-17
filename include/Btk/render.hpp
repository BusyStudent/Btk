#if !defined(_BTK_RENDERER_HPP_)
#define _BTK_RENDERER_HPP_
#include "pixels.hpp"
#include "rect.hpp"
#include "defs.hpp"
#include <list>
struct SDL_Window;

struct NVGcontext;//<NanoVG Context
namespace Btk{
    class Font;

    struct TextMetrics{
        float ascender;
        float descender;
        float h;//< The text's h
    };
    enum class Align:unsigned int;
    /**
     * @brief TextAlign from nanovg
     * 
     */
    enum class TextAlign:int{
        // Horizontal align
        Left     = 1<<0,	// Default, align text horizontally to left.
        Center 	 = 1<<1,	// Align text horizontally to center.
        Right 	 = 1<<2,	// Align text horizontally to right.
        // Vertical align
        Top 	 = 1<<3,	// Align text vertically to top.
        Middle	 = 1<<4,	// Align text vertically to middle.
        Bottom	 = 1<<5,	// Align text vertically to bottom.
        Baseline = 1<<6, // Default, align text vertically to baseline.
    };
    //TextAlign operator
    inline TextAlign operator |(TextAlign a,TextAlign b){
        return static_cast<TextAlign>(int(a) | int(b));
    }
    inline TextAlign operator +(TextAlign a,TextAlign b){
        return static_cast<TextAlign>(int(a) | int(b));
    }
    /**
     * @brief Abstruct Renderer
     * 
     */
    class BTKAPI Renderer{
        public:
            Renderer(SDL_Window *win);
            Renderer(const Renderer &) = delete;
            ~Renderer();

            //FIXME :Cliprect will cause render error
            //If you want to use it
            //Use render.save() and render.restore to protect
            //  the global context
            Rect get_cliprect();
            int  set_cliprect(){
                return set_cliprect(nullptr);
            }
            int  set_cliprect(const Rect &r){
                return set_cliprect(&r);
            }
            int  set_cliprect(const Rect *r);

           
            Rect get_viewport();
            int  set_viewport(){
                return set_viewport(nullptr);
            }
            int  set_viewport(const Rect &r){
                return set_viewport(&r);
            }
            int  set_viewport(const Rect *r);


            int  copy(const Texture &t,const Rect *src,const Rect *dst);
            int  copy(const Texture &t,const Rect &src,const Rect *dst);
            int  copy(const Texture &t,const Rect *src,const Rect &dst);
            int  copy(const Texture &t,const Rect &src,const Rect &dst);

            /**
             * @brief Destroy the renderer
             * 
             */
            void destroy();
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
            /**
             * @brief Dump a texture into pixbuf
             * 
             * @param texture The texture
             * @return Pixbuf
             */
            PixBuf  dump_texture(const Texture &texture);
            Texture clone_texture(const Texture &texture);
            /**
             * @brief Draw a image
             * 
             * @param dst The 
             * @param src 
             */
            void draw_image(const Texture&,const FRect *dst,const FRect *src);
        public:
            /**
             * @brief Begin the frame,Init the device
             * 
             */
            void begin();
            /**
             * @brief End the frame,swap the buffer
             * 
             */
            void end();
            /**
             * @brief Clear the screen
             * 
             * @param c 
             */
            void clear(Color c);
        public:
            //NanoVG Functions

            void begin_path();
            void close_path();

            void restore();
            void reset();
            void save();



            void fill();
            void fill_color(Color c);
            void fill_color(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
                fill_color({r,g,b,a});
            }


            void stroke();
            void stroke_color(Color c);
            void stroke_color(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
                stroke_color({r,g,b,a});
            }
            void stroke_width(float size);

            void move_to(float x,float y);
            void line_to(float x,float y);

            //NVG Graphics Path
            /**
             * @brief Create a rect subpath
             * 
             * @param x 
             * @param y 
             * @param w 
             * @param h 
             */
            void rect(float x,float y,float w,float h);
            void rect(const FRect &rect){
                this->rect(rect.x,rect.y,rect.w,rect.h);
            }
            void rounded_rect(float x,float y,float w,float h,float rad);
            void rounded_rect(const FRect &rect,float rad){
                this->rounded_rect(rect.x,rect.y,rect.w,rect.h,rad);
            }

            void show_path_caches();
            /**
             * @brief Draw text
             * 
             * @param x The Text Begin's x
             * @param y The Text's buttom
             * @param text 
             */
            void text(float x,float y,std::string_view text);
            void text(float x,float y,std::u16string_view text);
            /**
             * @brief Set the text's size
             * 
             * @param ptsize 
             */
            void text_size(float ptsize);

            TextMetrics font_metrics();
            /**
             * @brief Set Text Alignment
             * 
             * @param v_align Vertical Alignment(default Baseline)
             * @param h_align Horizontal Alignment(default Left)
             */
            void text_align(TextAlign align = TextAlign::Left | TextAlign::Baseline);
            void text_align(Align v_align,Align h_align);
            //Scissor
            void scissor(float x,float y,float w,float h);
            void scissor(const FRect &rect){
                scissor(rect.x,rect.y,rect.w,rect.h);
            }

            void reset_scissor();
        public:
            /**
             * @brief Flush the data
             * 
             */
            void swap_buffer();
        private:
            /**
             * @brief Free a texture
             * 
             * @param texture_id 
             */
            void free_texture(int texture_id);
            /**
             * @brief Update a texture pixels
             * 
             * @param texture_id 
             * @param pixels 
             */
            void update_texture(int texture_id,const void *pixels);
            /**
             * @brief Active the render context
             * 
             */
            void active();

            NVGcontext *nvg_ctxt = nullptr;//<NanoVG Context
            SDL_Window *window = nullptr;
            void *device = nullptr;//<Render device data

            Rect  viewport = {0,0,0,0};//< cached viewport
            FRect cliprect = {0,0,0,0};//< cached cliprect

            std::list<int> t_caches;//< Texture cache
            int max_caches = 20;//< Max cache
        friend class Texture;
    };

}

#endif // _BTK_RENDERER_HPP_
