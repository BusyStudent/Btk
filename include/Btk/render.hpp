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
     * @brief The renderer backend
     * 
     */
    enum class RendererBackend{
        OpenGL,
        Dx11,
        Software
    };
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
             * @brief Create a RGBA32 formatted texture
             * 
             * @param w 
             * @param h 
             * @return Texture 
             */
            Texture create(int w,int h);
            /**
             * @brief Load a texture from a file
             * 
             * @param fname The filename
             * @return Texture 
             */
            Texture load(std::string_view fname);
            Texture load(RWops &);
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
             * @param x 
             * @param y
             * @param w
             * @param h
             * @param angle
             */
            void draw_image(const Texture&,float x,float y,float w,float h,float angle = 0);
            void draw_image(const PixBuf& ,float x,float y,float w,float h,float angle = 0);
            void draw_image(const Texture& texture,const FRect &rect,float angle = 0){
                draw_image(texture,rect.x,rect.y,rect.w,rect.h,angle);
            }
            void draw_image(const PixBuf& pixbuf,const FRect &rect,float angle){
                draw_image(pixbuf,rect.x,rect.y,rect.w,rect.h,angle);
            }
            void draw_image(const Texture &,const FRect *src = nullptr,const FRect *dst = nullptr);
            void draw_image(const PixBuf  &,const FRect *src = nullptr,const FRect *dst = nullptr);
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
            /**
             * @brief Get the backend
             * 
             * @return RendererBackend 
             */
            RendererBackend backend() const;
            /**
             * @brief Get the logical drawable size
             * 
             * @return Size 
             */
            Size screen_size();
            /**
             * @brief Get the physical drawable size
             * 
             * @return Size 
             */
            Size output_size();
            /**
             * @brief For HDPI Device 
             * 
             * @return float 
             */
            float pixels_radio(){
                auto screen = screen_size();
                auto output = output_size();
                return float(output.w) / float(screen.w);
            }

            void set_target(const Texture &texture);
            
        public:
            //NanoVG Functions
            /**
             * @brief Get nanovg context
             * 
             * @return NVGcontext* 
             */
            NVGcontext *get() const noexcept{
                return nvg_ctxt;
            }

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

            void move_to(const FVec2 &vec){
                move_to(vec.x,vec.y);
            }
            void line_to(const FVec2 &vec){
                line_to(vec.x,vec.y);
            }

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
            /**
             * @brief Create a circle subpath
             * 
             * @param center_x 
             * @param center_y 
             * @param r 
             */
            void circle(float center_x,float center_y,float r);
            void circle(const FVec2 &vec,float r){
                circle(vec.x,vec.y,r);
            }
            /**
             * @brief Create a ellipse subpath
             * 
             * @param center_x 
             * @param center_y 
             * @param rx 
             * @param ry 
             */
            void ellipse(float center_x,float center_y,float rx,float ry);
            void ellipse(const FVec2 &vec,float rx,float ry){
                ellipse(vec.x,vec.y,rx,ry);
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
             * @brief Draw text(if the text's width > width)
             *        it will be drawed in next line
             * 
             * @param x 
             * @param y 
             * @param width 
             * @param text 
             */
            void textbox(float x,float y,float width,std::string_view text);
            void textbox(float x,float y,float width,std::u16string_view text);
            /**
             * @brief Get the size of the rendered string
             * 
             * @return FSize 
             */
            FSize text_size(std::string_view);
            FSize text_size(std::u16string_view);
            /**
             * @brief Set the text's size
             * 
             * @param ptsize 
             */
            void text_size(float ptsize);
            /**
             * @brief Get the sizeof the glyph
             * 
             * @return Size 
             */
            FSize glyph_size(char16_t );
            /**
             * @brief Get the height of the current font
             * 
             * @return float 
             */
            float font_height();
            TextMetrics font_metrics();
            /**
             * @brief Using the font
             * 
             * @param fontname The fontname
             * @return true On the font is exist
             * @return false On the font is not exist
             */
            bool use_font(std::string_view fontname) noexcept;
            /**
             * @brief Add font
             * 
             */
            void add_font(const char *fontname,const char *filename);
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
             * @brief Item for Texture
             * 
             */
            struct CachedItem{
                int texture;
                int w,h;
                bool used;
            };
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
             * @brief Get the texture's native handler
             * 
             * @param texture_id 
             * @param native_handle_ptr
             */
            void get_texture_handle(int texture_id,void *native_handle_ptr);
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
