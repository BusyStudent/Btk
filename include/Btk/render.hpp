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
    enum class Align:int;
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
    /**
     * @brief LineCap from nanovg
     * 
     */
    enum class LineCap:int{
        Butt,
        Round,
        Square,
        _Reserved1,
        _Reserved2,
    };
    /**
     * @brief LineJoin from nanovg
     * 
     */
    enum class LineJoin:int{
        _Reserved1,
        Round,
        _Reserved2,
        Bevel,
        Miter,
    };

    enum class PathWinding{
        CCW = 1, //solid shapes
        CW = 2, //Hole

        Solid = CCW,
        Hole = CW
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
     * @brief Same defs in nanovg
     * 
     */
    class RendererPaint{
        public:
            RendererPaint() = default;
            RendererPaint(const RendererPaint &) = default;
            ~RendererPaint() = default;

            RendererPaint &operator =(RendererPaint &) = default;
            RendererPaint &operator =(RendererPaint &&) = default;
        private:
            float xform[6];
            float extent[2];
            float radius;
            float feather;
            GLColor innerColor;
            GLColor outerColor;
            TextureID image;
    };
    /**
     * @brief TextureType from nanovg
     * 
     */
    enum class TextureType:int{
        Alpha = 0x01,
        RGBA32 = 0x02
    };
    enum class LockFlag:Uint32{
        Read = 1 << 1,
        Write = 1 << 2,
        ReadAndWrite = Read | Write
    };
    BTK_FLAGS_OPERATOR(LockFlag,Uint32);
    /**
     * @brief Info of the renderer devices
     * 
     */
    struct RebdererDeviceInfo{
        //Limits
        Uint32 *pixels_formats;
        Size max_texture_size;
        Uint32 max_textures;
        //Supports
        bool native_lock_op;
    };
    /**
     * @brief Abstruct Graphics Device
     * 
     */
    class BTKAPI RendererDevice{
    public:
        using Context = NVGcontext*;
        static constexpr auto Read = LockFlag::Read;
        static constexpr auto Write = LockFlag::Write;
        static constexpr auto ReadAndWrite = LockFlag::ReadAndWrite;


        virtual ~RendererDevice();
        /**
         * @brief Create a nanovg context object
         * 
         * @return Context* 
         */
        virtual Context create_context() = 0;
        virtual void    destroy_context(Context) = 0;
        //Frame operations
        /**
         * @brief Begin a frame,note it will clear the state of the context
         * 
         * @param ctxt 
         * @param w Logical's Width
         * @param h Logical's Height
         * @param pixel_ratio Physical's w / Logical's w
         */
        virtual void begin_frame(Context ctxt,
                                 float w,
                                 float h,
                                 float pixel_ratio);
        /**
         * @brief Like begin_frame,but it didnot clear the state of the context
         * 
         * @param ctxt 
         * @param w Logical's Width
         * @param h Logical's Height
         * @param pixel_ratio Physical's w / Logical's w
         */
        static  void begin_frame_ex(Context ctxt,
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
        virtual void*     lock_texture(
            Context ctxt,
            TextureID id,
            const Rect *r,
            LockFlag flag
        );
        virtual void      unlock_texture(
            Context ctxt,
            TextureID id,
            void *pixels
        );
        virtual bool      configure_texture(
            Context ctxt,
            TextureID id,
            const TextureFlags *p_flags
        );
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
        bool set_texture_flags(Context ctxt,TextureID id,TextureFlags flags){
            return configure_texture(ctxt,id,&flags);
        }
        RendererBackend backend() const noexcept{
            return _backend;
        }
        /**
         * @brief Create a texture from PixelBuf
         * 
         * @param ctxt 
         * @param surf 
         * @param flags 
         * @return TextureID 
         */
        TextureID create_texture_from(
            Context ctxt,
            SDL_Surface *surf,
            TextureFlags flags
        );
        TextureID create_texture_from(
            Context ctxt,
            PixBufRef    buf,
            TextureFlags flags
        ){
            return create_texture_from(
                ctxt,
                buf.get(),
                flags
            );
        }
        template<class ...Args>
        void set_error(Args &&...args){
            if constexpr(sizeof... (Args) == 1){
                clear_error();
                _err.append(std::forward<Args>(args)...);
            }
            else{
                _err.clear();
                _err.append_fmt(std::forward<Args>(args)...);
            }
        }
        void clear_error(){
            _err.clear();
        }
        const u8string &get_error() const noexcept{
            return _err;
        }
    protected:
        void set_backend(RendererBackend bac){
            _backend = bac;
        }
    private:
        RendererBackend _backend = RendererBackend::Unknown;
        u8string        _err;
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
            ~Renderer(){
                destroy();
            }
            /**
             * @brief Destroy the renderer
             * 
             */
            void destroy();
            /**
             * @brief Create a Texture from Pixbuf
             * 
             * @param pixbuf The Pixbuffer
             * @param flags The texture flags
             * @return Texture The texture
             */
            Texture create_from(PixBufRef pixbuf,TextureFlags flags = TextureFlags::Linear);
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
            void draw_image(TextureRef tex,float x,float y,float w,float h,float angle = 0){
                _draw_image(tex.get(),x,y,w,h,angle);
            }
            void draw_image(PixBufRef  buf,float x,float y,float w,float h,float angle = 0);
            void draw_image(TextureRef texture,const FRect &rect,float angle = 0){
                _draw_image(texture.get(),rect.x,rect.y,rect.w,rect.h,angle);
            }
            void draw_image(PixBufRef  buf,const FRect &rect,float angle){
                draw_image(buf,rect.x,rect.y,rect.w,rect.h,angle);
            }
            void draw_image(TextureRef tex,const FRect *src = nullptr,const FRect *dst = nullptr){
                _draw_image(tex.get(),src,dst);
            }
            void draw_image(PixBufRef  buf,const FRect *src = nullptr,const FRect *dst = nullptr);

            void draw_box(const FRect &r,Color c);
            void draw_rect(const FRect &r,Color c);
            void draw_line(float x1,float y1,float x2,float y2,Color c);
            void draw_line(const FVec2 &beg,const FVec2 &end,Color c){
                return draw_line(beg.x,beg.y,end.x,end.y,c);
            }

            void draw_rounded_rect(const FRect &r,float rad,Color c);
            void draw_rounded_box(const FRect &r,float rad,Color c);
            void draw_ellipse(float x,float y,float rx,float ry,Color c);
            void fill_ellipse(float x,float y,float rx,float ry,Color c);
            void draw_circle(float x,float y,float r,Color c);
            void fill_circle(float x,float y,float r,Color c);

            /**
             * @brief Draw a circle
             * 
             * @param vec 
             * @param r 
             * @param c 
             */
            void draw_circle(const FVec2 &vec,float r,Color c){
                draw_circle(vec.x,vec.y,r,c);
            }
            /**
             * @brief Fill a circle
             * 
             * @param vec 
             * @param r 
             * @param c 
             */
            void fill_circle(const FVec2 &vec,float r,Color c){
                fill_circle(vec.x,vec.y,r,c);
            }
            void draw_text(float x,float y,u8string_view txt,Color c);

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
                return device()->backend();
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
            void fill_color(const GLColor &c);
            void fill_color(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
                fill_color(Color{r,g,b,a});
            }
            void fill_paint(const RendererPaint &paint);

            void stroke();
            void stroke_color(Color c);
            void stroke_color(const GLColor &c);
            void stroke_color(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
                stroke_color(Color{r,g,b,a});
            }

            void stroke_width(float size);
            void stroke_paint(const RendererPaint &paint);
            /**
             * @brief Move to
             * 
             * @param x 
             * @param y 
             */
            void move_to(float x,float y);
            /**
             * @brief Line to
             * 
             * @param x 
             * @param y 
             */
            void line_to(float x,float y);
            /**
             * @brief Quadratic bezier
             * 
             * @param cx The control point x
             * @param cy The control point y
             * @param x The new point x
             * @param y The new point y
             */
            void quad_to(float cx,float cy,float x,float y);
            /**
             * @brief Arc to
             * 
             * @param x1 
             * @param y1 
             * @param x2 
             * @param y2 
             * @param radius 
             */
            void arc_to(float x1,float y1,float x2,float y2,float radius);
            /**
             * @brief Cubic bezier
             * 
             * @param c1x The control point 1 x
             * @param c1y The control point 1 y
             * @param c2x The control point 2 x
             * @param c2y The control point 2 y
             * @param x The new point x
             * @param y The new point y
             */
            void bezier_to(float c1x,float c1y,float c2x,float c2y,float x,float y);

            void move_to(const FVec2 &vec){
                move_to(vec.x,vec.y);
            }
            void line_to(const FVec2 &vec){
                line_to(vec.x,vec.y);
            }
            void quad_to(const FVec2 &control_p,const FVec2 &point){
                quad_to(control_p.x,control_p.y,point.x,point.y);
            }
            void bezier_to(const FVec2 &control1,const FVec2 &control2,const FVec2 &point){
                bezier_to(
                    control1.x,
                    control1.y,
                    control2.x,
                    control2.y,
                    point.x,
                    point.y
                );
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
            void triangle(float x1,float y1,float x2,float y2,float x3,float y3);
            void triangle(const FVec2 &a,const FVec2 &b,const FVec2 &c){
                triangle(a.x,a.y,b.x,b.y,c.x,c.y);
            }

            // template<class Container>
            // void polygon(const Container &c){
            //     move_to
            //     for(auto p:c){

            //     }
            // }


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

            FRect text_bounds(float x,float y,u8string_view str);
            /**
             * @brief Get the size of the rendered string
             * 
             * @return FSize 
             */
            FSize text_size(u8string_view);
            FSize text_size(u16string_view);
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
            void text_align(Align align){
                text_align(TextAlign(align));
            }
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
            void set_antialias(bool val = true);
            void set_alpha(float alpha = 1.0f);
            void set_pathwinding(PathWinding v);
            void set_linecap(LineCap c);
            void set_linejoin(LineJoin c);
            void set_miterlimit(float limit);
        public:
            //Paint
            /**
             * @brief Make a image pattern
             * 
             * @param r The image rect
             * @param angle The angle to te
             * @param tex   The texture 
             * @param alpha The aplha
             * @return RendererPaint 
             */
            RendererPaint image_pattern(const FRect &r,float angle,TextureID tex,float alpha);
            /**
             * @brief 
             * 
             * @param sx South X
             * @param sy South Y
             * @param ex East X
             * @param ey East Y
             * @param in The begin color
             * @param out The end color
             * @return RendererPaint 
             */
            RendererPaint linear_gradient(float sx,float sy,float ex,float ey,GLColor in,GLColor out);
            /**
             * @brief 
             * 
             * @param cx The center x
             * @param cy The center y
             * @param inr 
             * @param outr 
             * @param in 
             * @param out 
             * @return RendererPaint 
             */
            RendererPaint radial_gradient(float cx,float cy,float inr,float outr,GLColor in,GLColor out);
        public:
            //Transform
            void scale(float x_factor,float y_factor);
            void translate(float x,float y);
            void rotate(float angel);
            void skew_x(float angle);
            void skew_y(float angle);
            void reset_transform();
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
            auto context() const noexcept -> NVGcontext *{
                return nvg_ctxt;
            }
        private:
            //Internal
            void _draw_image(TextureID tex,float x,float y,float w,float h,float angle = 0,float alpha = 1);
            void _draw_image(TextureID texture,const FRect *src,const FRect *dst);
            /**
             * @brief Item for Texture
             * 
             */
            struct CachedItem{
                int tex = -1;
                int w = -1;
                int h = -1;
                bool used = false;
            };
            /**
             * @brief Try to find a cache
             * 
             * @param req_w Request width
             * @param req_h Reqyest height
             * @return nullptr on failure
             */
            CachedItem *find_cache(int req_w,int req_h);
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
            // TextureID update(texture);
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
            bool set_texture_flags(int texture_id,TextureFlags flags){
                return device()->set_texture_flags(nvg_ctxt,texture_id,flags);
            }
            TextureFlags get_texture_flags(int texture_id){
                return device()->texture_flags(nvg_ctxt,texture_id);
            }

            NVGcontext *nvg_ctxt = nullptr;//<NanoVG Context
            Device     *_device = nullptr;//<Render device data

            std::list<CachedItem> cached_texs;//< Texture cache
            int max_caches = 20;//< Max cache

            bool is_drawing = false;//< Is nanovg Has BeginFrame
            bool free_device = false;
        friend class Texture;
        friend class TextureRef;
        friend class GLCanvas;
    };

}

#endif // _BTK_RENDERER_HPP_
