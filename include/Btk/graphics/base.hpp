#if !defined(_BTK_GRAPHICS_BASE_HPP_)
#define _BTK_GRAPHICS_BASE_HPP_

#include "../pixels.hpp"
#include "../defs.hpp"

namespace Btk {
    class Renderer;
    /**
     * @brief Paths Container for Renderer
     * 
     */
    class RendererPaths {
        public:
            RendererPaths() = default;
            RendererPaths(const RendererPaths&) = default;
            RendererPaths(RendererPaths&&) = default;
            ~RendererPaths() = default;

            void clear();
            void begin_path();
            void move_to(const Point&);
            void line_to(const Point&);
            void close_path();

            void render(Renderer&) const;
            void translate(float x,float y);
            // void transform(const Matrix&);
        private:
            enum : Uint8 {
                MoveTo,
                LineTo,
                QuadTo,
                CubicTo,
                ArcTo,
                ClosePath,
                BeginPath,
                NumCommands
            };
            struct Command {
                Uint8 type;
                union {
                    struct {
                        float x, y;
                    } move_to;
                    struct {
                        float x, y;
                    } line_to;
                    struct {
                        float x1, y1, x2, y2;
                    } quad_to;
                    struct {
                        float x1, y1, x2, y2, x3, y3;
                    } cubic_to;
                    struct {
                        float x, y, rx, ry, angle, largeArc, sweep;
                    } arc_to;
                } data;
            };

            std::vector<Command> cmds;
        friend class Renderer;
    };
    /**
     * @brief Gradient structure for describe it
     * 
     */
    class BTKAPI Gradient {
        public:
            enum Mode : Uint8 {
                Relative,
                Absolute
            };
            enum Type : Uint8 {
                Linear,
                Radial,
            };
            enum Spread : Uint8 {
                Pad,
                Reflect,
                Repeat
            };
            struct StopPoint {
                float position;
                Color color;
            };
        public:
            /**
             * @brief Construct a new Gradient object with Relative Coordinates
             * 
             */
            Gradient(){
                _mode = Relative;
                _area.minx = 0.0f;
                _area.miny = 0.0f;
                _area.maxx = 1.0f;
                _area.maxy = 1.0f;
            }
            /**
             * @brief Construct a new Gradient object with Absolute Coordinates
             * 
             * @param minx 
             * @param miny 
             * @param maxx 
             * @param maxy 
             */
            Gradient(float minx,float miny,float maxx,float maxy){
                _mode = Absolute;
                _area.minx = minx;
                _area.miny = miny;
                _area.maxx = maxx;
                _area.maxy = maxy;
            }
            
            Gradient(const Gradient&);
            Gradient(Gradient&&);
            ~Gradient();

            //Bultin Preset Gradients support
            enum Preset : int;
            #ifdef BTK_USE_BUILTIN_GRADIENTS
            #include "./gradients_enum.inl"
            #endif
            Gradient(Preset);

            /**
             * @brief Add a color stop to the gradient
             * 
             * @param x The x position, normalized between 0 and 1
             * @param y The y position, normalized between 0 and 1
             * @param c The color
             */
            size_t add_color(float pos,Color c){
                pos = clamp(pos,0.0f,1.0f);

                StopPoint p;
                p.position = pos;
                p.color = c;

                colors.push_back(p);
                return colors.size() - 1;
            }
            /**
             * @brief Set the Coordinates mode
             * 
             * @param mode 
             */
            void set_mode(Mode mode){
                _mode = mode;
            }
            void set_type(Type type){
                _type = type;
            }
            void set_spread(Spread spread){
                _spread = spread;
            }
            void set_bounds(float minx,float miny,float maxx,float maxy){
                _area.minx = minx;
                _area.miny = miny;
                _area.maxx = maxx;
                _area.maxy = maxy;
            }
            void set_bounds(const FBounds& r){
                _area = r;
            }
        protected:
            union Data {
                FPoint radial;//< Center Point for Radial Gradient
            };

            std::vector<StopPoint> colors;

            Data    _data; //Extra data to LinearGradient or RadialGradient
            Spread  _spread = Pad;
            Mode    _mode;
            Type    _type;
            FBounds _area; //Gradient work area
        friend class Renderer;
    };
    /**
     * @brief Linear Gradient
     * 
     */
    class LinearGradient : public Gradient {
        public:
            LinearGradient(){
                _type = Gradient::Linear;
            }
            LinearGradient(float minx,float miny,float maxx,float maxy) 
                : Gradient(minx,miny,maxx,maxy)
            {
                _type = Gradient::Linear;
            }
            LinearGradient(const LinearGradient&) = default;
            LinearGradient(LinearGradient&&) = default;
            ~LinearGradient() = default;
        friend class Renderer;
    };
    using GradientRef = const Gradient &;
    
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
    /**
     * @brief The lock data from texture
     * @todo Using this struct to instead of void*
     */
    struct LockData{
        void *pixels;
        int pitch;
        //< Drvier specific data
        void *priv;
    };

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
                defer_clear();
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
            /**
             * @brief Release the texture by using cmd queue
             * 
             */
            void defer_clear();

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
     * @brief Brush content type
     * 
     */
    enum class BrushType{
        Color,
        Image,
        Gradient,
    };
    /**
     * @brief Fill content abstraction
     * 
     */
    class BTKAPI Brush{
        public:
            Brush() = default;
            Brush(const Brush &);
            ~Brush(){
                clear();
            }

            Brush(PixBufRef ref){
                set_image(ref);
            }
            Brush(Color c){
                set_color(c);
            }
            Brush(GradientRef g){
                set_gradient(g);
            }
            Brush(Gradient &&g){
                set_gradient(std::move(g));
            }

            void clear();
            void assign(const Brush &);
            void assign(Brush &&);

            void set_image(PixBufRef image){
                clear();
                _type = BrushType::Image;
                new (&data.image.buf) PixBuf(image);
                data.image.flags = TextureFlags(0);
            }
            void set_color(Color c){
                clear();
                _type = BrushType::Color;
                data.color = c;
            }
            void set_gradient(GradientRef g){
                clear();
                _type = BrushType::Gradient;
                new (&data.gradient) Gradient(g);
            }
            void set_gradient(Gradient && g){
                clear();
                _type = BrushType::Gradient;
                new (&data.gradient) Gradient(std::move(g));
            }

            BrushType type() const noexcept{
                return _type;
            }
            Color color() const{
                return data.color;
            }
            PixBufRef image() const{
                return data.image.buf;
            }
            GradientRef graident() const{
                return data.gradient;
            }

            Brush &operator =(const Brush &brush){
                if(&brush == this){
                    return *this;
                }
                assign(brush);
                return *this;
            }
            Brush &operator =(Brush &&brush){
                if(&brush == this){
                    return *this;
                }
                assign(std::move(brush));
                return *this;
            }

            //Cast
            operator Color() const;
        private:
            union Data{
                Data(){}
                ~Data(){}

                Color color = Color(0,0,0,0);
                Gradient gradient;

                struct {
                    TextureFlags flags;
                    PixBuf buf;
                    float alpha;
                }image;

            }data;
            BrushType _type = BrushType::Color;
    };
    using BrushRef = const Brush &;
}


#endif // _BTK_GRAPHICS_BASE_HPP_
