#if !defined(_BTK_RENDERER_HPP_)
#define _BTK_RENDERER_HPP_
#include "pixels.hpp"
#include "string.hpp"
#include "rect.hpp"
#include "defs.hpp"
#include <vector>
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
        //Vertical center
        VCenter = Middle,
        // Horizontal center
        HCenter = Center
    };
    //TextAlign operator
    // inline TextAlign operator |(TextAlign a,TextAlign b){
    //     return static_cast<TextAlign>(int(a) | int(b));
    // }
    // inline TextAlign operator +(TextAlign a,TextAlign b){
    //     return static_cast<TextAlign>(int(a) | int(b));
    // }
    BTK_FLAGS_OPERATOR(TextAlign,int);
    /**
     * @brief The renderer backend
     * 
     */
    enum class RendererBackend{
        Unknown = 0,
        OpenGL,
        Metail,
        Dx11,
        Software
    };
    /**
     * @brief Glyph position from nanovg
     * 
     */
    struct GlyphPosition{
        char32_t glyph;     // The glyph 
        const char* str;	// Position of the glyph in the input string.
        float x;			// The x-coordinate of the logical glyph position.
        float minx, maxx;	// The bounds of the glyph shape.
    };
    /**
     * @brief Abstruct Graphics Device
     * 
     */
    class BTKAPI RendererDevice{
    public:
        using Context = NVGcontext*;

        virtual ~RendererDevice(){};
        /**
         * @brief Create a nanovg context object
         * 
         * @return Context* 
         */
        virtual Context create_context() = 0;
        virtual void    destroy_context(Context) = 0;
        //Frame operations
        virtual void begin_frame(Context ctxt,
                                 float w,
                                 float h,
                                 float pixel_ratio);
        virtual void cancel_frame(Context ctxt);
        virtual void end_frame(Context ctxt);
        //Buffer or Screen
        /**
         * @brief Clear the screen
         * 
         * @param bg The background color
         */
        virtual void clear_buffer(Color bg) = 0;
        virtual void swap_buffer(){};
        /**
         * @brief Set the viewport object
         * 
         * @param r The viewport(nullptr on reset)
         */
        virtual void set_viewport(const Rect *r) = 0;
        /**
         * @brief Set the target object
         * 
         * @param ctxt 
         * @param id 
         */
        virtual void set_target(Context ctxt,TextureID id) = 0;
        /**
         * @brief Reset
         * 
         * @param ctxt 
         */
        virtual void reset_target(Context ctxt) = 0;
        //Texture
        virtual TextureID create_texture(Context ctxt,
                                         int w,
                                         int h,
                                         TextureFlags,
                                         const void *pix = nullptr);
        virtual TextureID clone_texture(Context ctxt,TextureID) = 0;
        virtual void      destroy_texture(Context ctxt,TextureID t);
        virtual void      update_texture(Context ctxt,
                                         TextureID t,
                                         const Rect *r,
                                         const void *pixels);
        /**
         * @brief Query texture information
         * 
         * @param ctxt The render context
         * @param id The texture id
         * @param p_size The pointer to size(could be nullptr)
         * @param p_handle The pointer to native_handle(could be nullptr)
         */
        virtual bool      query_texture(Context ctxt,
                                        TextureID id,
                                        Size *p_size,
                                        void *p_handle,
                                        TextureFlags *p_flags) = 0;
        /**
         * @brief Get output size
         * 
         * @param p_logical_size 
         * @param p_physical_size 
         * @return true 
         * @return false 
         */
        virtual bool output_size(
            Size *p_logical_size,
            Size *p_physical_size
        ) = 0;
        /**
         * @brief Get logical output size
         * 
         * @return Size 
         */
        Size logical_size(){
            Size s;
            output_size(&s,nullptr);
            return s;
        }
        /**
         * @brief Get physical output size
         * 
         * @return Size 
         */
        Size physical_size(){
            Size s;
            output_size(nullptr,&s);
            return s;
        }
        /**
         * @brief Get texture size
         * 
         * @param ctxt 
         * @param id 
         * @return Size 
         */
        Size texture_size(Context ctxt,TextureID id){
            Size s;
            query_texture(ctxt,id,&s,nullptr,nullptr);
            return s;
        }
        /**
         * @brief Get texture flags
         * 
         * @param ctxt 
         * @param id 
         * @return TextureFlags 
         */
        TextureFlags texture_flags(Context ctxt,TextureID id){
            TextureFlags flags;
            query_texture(ctxt,id,nullptr,nullptr,&flags);
            return flags;
        }
        /**
         * @brief Get texture native handle
         * 
         * @param ctxt 
         * @param id 
         * @param p_handle 
         */
        void texture_native_handle(Context ctxt,TextureID id,void *p_handle){
            query_texture(ctxt,id,nullptr,p_handle,nullptr);
        }
        RendererBackend backend() const noexcept{
            return _backend;
        }
    protected:
        void set_backend(RendererBackend bac){
            _backend = bac;
        }
    private:
        RendererBackend _backend = RendererBackend::Unknown;
    };
    /**
     * @brief Abstruct Renderer
     * 
     */
    class BTKAPI Renderer{
        public:
            using Device = RendererDevice;
            /**
             * @brief Construct a new Renderer object
             * 
             * @param dev The device
             * @param owned Shoud we delete the device?
             */
            Renderer(Device &dev,bool owned = false);
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
             * @param flags The texture flags
             * @return Texture The texture
             */
            Texture create_from(const PixBuf &pixbuf,TextureFlags flags = TextureFlags::Linear);
            /**
             * @brief Create a from handle object
             * 
             * @param p_handle The pointer to native handler
             * @param w The texture's width
             * @param h The texture's height
             * @param flags The texture flags
             * @return Texture 
             */
            Texture create_from_handle(
                const void *p_handle,
                int w,
                int h,
                TextureFlags flags = TextureFlags::Linear
            );
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
             * @param flas texture flags
             * @return Texture 
             */
            Texture create(int w,int h,TextureFlags flags = TextureFlags::Linear);
            /**
             * @brief Load a texture from a file
             * 
             * @param fname The filename
             * @return Texture 
             */
            Texture load(u8string_view fname,TextureFlags flags = TextureFlags::Linear);
            Texture load(RWops &,TextureFlags flags = TextureFlags::Linear);
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
            int  text(Font &,int x,int y,Color c,u8string_view u8);
            int  text(Font &,int x,int y,Color c,u16string_view u16);
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
            int  vtext(Font &,int x,int y,Color c,u8string_view fmt,...);
            int  vtext(Font &,int x,int y,Color c,u16string_view fmt,...);
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
            /**
             * @brief Draw a circle
             * 
             * @param vec 
             * @param r 
             * @param c 
             */
            void draw_circle(const FVec2 &vec,float r,Color c){
                begin_path();
                circle(vec,r);
                stroke_color(c);
                stroke();
            }
            void draw_box(const FRect &r,Color c){
                begin_path();
                rect(r);
                fill_color(c);
                fill();
            }
            void draw_rect(const FRect &r,Color c){
                begin_path();
                rect(r);
                stroke_color(c);
                stroke();
            }
            /**
             * @brief Fill a circle
             * 
             * @param vec 
             * @param r 
             * @param c 
             */
            void fill_circle(const FVec2 &vec,float r,Color c){
                begin_path();
                circle(vec,r);
                fill_color(c);
                fill();
            }
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
            void clear(Color c){
                device()->clear_buffer(c);
            }
            void clear(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
                clear({r,g,b,a});
            }
            void flush();
            /**
             * @brief Get the backend
             * 
             * @return RendererBackend 
             */
            RendererBackend backend() const{
                device()->backend();
            }
            /**
             * @brief Get the logical drawable size
             * 
             * @return Size 
             */
            Size screen_size(){
                return device()->logical_size();
            }
            /**
             * @brief Get the physical drawable size
             * 
             * @return Size 
             */
            Size output_size(){
                return device()->physical_size();
            }
            /**
             * @brief For HDPI Device 
             * 
             * @return float 
             */
            float pixels_ratio(){
                auto screen = screen_size();
                auto output = output_size();
                return float(output.w) / float(screen.w);
            }
            /**
             * @brief Set the target object
             * 
             * @param texture The texture you want to draw
             */
            void set_target(Texture &texture);
            /**
             * @brief Reset it back to the screen
             * 
             */
            void reset_target();
            /**
             * @brief Make the Context current
             * 
             */
            void make_current();
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
            void begin_frame(float w,float h,float ratio);
            void end_frame();

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
            void text(float x,float y,u8string_view text);
            void text(float x,float y,u16string_view text);
            void text(const FVec2 &p,u8string_view text){
                this->text(p.x,p.y,text);
            }
            void text(const FVec2 &p,u16string_view text){
                this->text(p.x,p.y,text);
            }
            /**
             * @brief Draw text(if the text's width > width)
             *        it will be drawed in next line
             * 
             * @param x 
             * @param y 
             * @param width 
             * @param text 
             */
            void textbox(float x,float y,float width,u8string_view text);
            void textbox(float x,float y,float width,u16string_view text);
            /**
             * @brief Get the size of the rendered string
             * 
             * @return FSize 
             */
            FSize text_size(u8string_view);
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
            FSize  glyph_size(char16_t );
            /**
             * @brief Get position of glyph
             * 
             * @note It is low level a operation
             * 
             * @param x The text's x
             * @param y The text's y
             * @param text The utf8 encoding string
             * @param callback The callback(return false to stop)
             * @param user The userdata
             * @return size_t 
             */
            size_t glyph_position(
                float x,float y,
                u8string_view text,
                bool (*callback)(const GlyphPosition &,void *user),
                void *user    
            );
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
            bool use_font(u8string_view fontname) noexcept;
            bool use_font(const Font &font);
            /**
             * @brief Add font
             * 
             */
            void add_font(const char *fontname,const char *filename);
            /**
             * @brief Get current font
             * 
             * @return Font 
             */
            Font cur_font();
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
            void intersest_scissor(float x,float y,float w,float h);
            void intersest_scissor(const FRect &rect){
                intersest_scissor(rect.x,rect.y,rect.w,rect.h);
            }
            void reset_scissor();

        public:
            //Transform
            void scale(float x_factor,float y_factor);
            void translate(float x,float y);
            void rotate(float angel);
            void skew_x(float angle);
            void skew_y(float angle);
        public:
            /**
             * @brief Flush the data
             * 
             */
            void swap_buffer();
            /**
             * @brief Get device
             * 
             * @return RendererDevice& 
             */
            RendererDevice *device() const noexcept{
                return _device;
            }
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
            void destroy_texture(int texture_id){
                device()->destroy_texture(nvg_ctxt,texture_id);
            }
            int  clone_texture(int texture_id){
                return device()->clone_texture(nvg_ctxt,texture_id);
            }
            PixBuf dump_texture(int texture_id);
            /**
             * @brief Update a texture pixels
             * 
             * @param texture_id 
             * @param pixels 
             */
            void update_texture(int texture_id,const void *pixels){
                device()->update_texture(nvg_ctxt,texture_id,nullptr,pixels);
            }
            void update_texture(int texture_id,const Rect&r,const void *pixels){
                device()->update_texture(nvg_ctxt,texture_id,&r,pixels);
            }
            /**
             * @brief Get the texture's native handler
             * 
             * @param texture_id 
             * @param native_handle_ptr
             */
            void get_texture_handle(int texture_id,void *native_handle_ptr){
                device()->texture_native_handle(nvg_ctxt,texture_id,native_handle_ptr);
            }
            void set_texture_flags(int texture_id,TextureFlags);
            TextureFlags get_texture_flags(int texture_id){
                return device()->texture_flags(nvg_ctxt,texture_id);
            }

            NVGcontext *nvg_ctxt = nullptr;//<NanoVG Context
            Device     *_device = nullptr;//<Render device data
            
            Rect  viewport = {0,0,0,0};//< cached viewport
            FRect cliprect = {0,0,0,0};//< cached cliprect

            std::list<int> t_caches;//< Texture cache
            int max_caches = 20;//< Max cache

            bool is_drawing = false;//< Is nanovg Has BeginFrame
            bool free_device = false;
        friend class Texture;
    };

}

#endif // _BTK_RENDERER_HPP_
