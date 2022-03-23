#if !defined(_BOX_FONT_HPP_)
#define _BOX_FONT_HPP_
#include <cstddef>
#include "string.hpp"
#include "pixels.hpp"
#include "defs.hpp"
namespace Btk{
    /**
     * @brief FontStyle from SDL_ttf
     * 
     */
    enum class FontStyle:int{
        Normal = 0,
        Bold = 1 << 0,
        Italic = 1 << 1,
        Underline = 1 << 2,
        Strikethrough = 1 << 3
    };
    using FontID = int;
    inline constexpr FontID FontInvalidID = -1;
    /**
     * @brief Font Class
     * 
     */
    class BTKAPI Font{
        public:
            //empty font
            Font() = default;
            Font(const Font &font);
            Font(Font &&f){
                font = f.font;
                f.font = nullptr;
            };
            /**
             * @brief Construct a new Font object
             * 
             * @param fontname Font name
             * @param ptsize Font ptsize
             */
            Font(u8string_view fontname,float ptsize);
            ~Font(){
                close();
            }
            /**
             * @brief Render solid text
             * 
             * @note This one is fastest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            // PixBuf render_solid(u8string_view text,Color color);
            // PixBuf render_solid(u16string_view text,Color color);
            /**
             * @brief Render shaded text
             * 
             * @note This one is middle speed
             * @param text Your text
             * @param fg Text color
             * @param bg BackGround color
             * @return PixBuf 
             */
            // PixBuf render_shaded(u8string_view text,Color fg,Color bg);
            // PixBuf render_shaded(u16string_view text,Color fg,Color bg);
            /**
             * @brief Render blended text
             * 
             * @note This one is slowest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            // PixBuf render_blended(u8string_view text,Color color);
            // PixBuf render_blended(u16string_view text,Color color);
            /**
             * @brief Get Kerning Size in two chars
             * 
             * @param prev_ch The first char
             * @param ch The next char
             * @return The kerning
             */
            int kerning_size(char32_t prev_ch,char32_t ch) const;
            /**
             * @brief Check the font has the glyph
             * 
             * @param ch The utf16 char
             * @return true 
             * @return false 
             */
            bool has_glyph(char32_t ch) const;
            /**
             * @brief Get font height
             * 
             * @return float 
             */
            float height() const;
            /**
             * @brief Get font ptsize
             * 
             * @return int 
             */
            float ptsize() const noexcept{
                return ptsize_;
            }
            float blur() const noexcept{
                return blur_;
            }
            /**
             * @brief Get font refcount
             * 
             * @return int 
             */
            int  refcount() const noexcept;
            /**
             * @brief Set the font ptsize
             * 
             * @param new_ptsize New ptsize
             */
            void set_ptsize(float new_ptsize){
                ptsize_ = new_ptsize;
            }
            void set_spacing(float spacing){
                spacing_ = spacing;
            }
            void set_blur(float blur){
                blur_ = blur;
            }
            /**
             * @brief Get The text's w and h
             * 
             * @param text The text
             * @return w and h (-1 if failed)
             */
            FSize size(u8string_view text) const;
            FSize size(u16string_view text) const;
            /**
             * @brief Close font
             * 
             */
            void close();
            /**
             * @brief Open font by its name
             * 
             * @param fontname Font name
             * @param ptsize Font ptsize
             * @param idx The face index(default 0)
             */
            void open(u8string_view fontname,float ptsize,Uint32 idx = 0);
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             * @param idx The face index(default 0)
             * 
             */
            void openfile(u8string_view filename,float ptsize,Uint32 idx = 0);
            /**
             * @brief Clone a font
             * 
             * @return Font 
             */
            Font clone() const;
            /**
             * @brief Get this font's style
             * 
             * @return FontStyle 
             */
            FontStyle style() const;
            /**
             * @brief Set the style 
             * 
             * @param style The style you want to set
             */
            void  set_style(FontStyle style);
            /**
             * @brief Get font family name
             * 
             * @return family name
             */
            u8string family() const;
            u8string style_name() const;
            FontID id() const;
            /**
             * @brief Assign font
             * 
             * @return *this 
             */
            Font &operator =(const Font &);
            Font &operator =(Font &&);
            bool empty() const noexcept{
                return font == nullptr;
            };
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             * @return Font 
             */
            static Font FromFile(u8string_view filename,float ptsize);
            static Font FromID(FontID id,float ptsize);
            static Font DefaultFont(){
                return FromID(0,12);
            }
            static void Init();
            static void Quit();
        private:
            Font(void *font_ptr);

            void *font = nullptr;
            float ptsize_ = -1;
            float spacing_ = 0;
            float blur_ = 0;
        friend class Renderer;
    };
    //TODO
    class BTKAPI FontSet{
        #ifdef __gnu_linux__
        public:
            /**
             * @brief Construct a new Font Set object
             * 
             * @param pat FcPattern
             * @param objset FcObjectSet
             * @param fontset FcFontSet
             */
            FontSet(void *pat,void *objset,void *fontset):
                fc_pat(pat),fc_objset(objset),fc_fontset(fontset){}
            FontSet(FontSet &&set){
                fc_pat = set.fc_pat;
                fc_objset = set.fc_objset;
                fc_fontset = set.fc_fontset;

                set.fc_pat = nullptr;
                set.fc_objset = nullptr;
                set.fc_fontset = nullptr;
            }
        public:
            /**
             * @brief Font in FontSet
             * 
             */
            struct BTKAPI Font{
                /**
                 * @note The return of the methods are all reference,
                 *       If you want to use it after destroying the fontset,
                 *       Please make a copy of the string_view
                 */
                u8string_view family() const;
                u8string_view style() const;
                u8string_view file() const;
                void  *font;
            };
            /**
             * @brief Iterator of Fontset
             * 
             */
            struct Iterator{
                Iterator() = default;
                Iterator(const Iterator &) = default;

                bool operator ==(const Iterator &i) const{
                    return i.font.font == font.font;
                }
                bool operator !=(const Iterator &i) const{
                    return i.font.font != font.font;
                }
                Font *operator ->(){
                    return &font;
                }
                Font &operator * (){
                    return font;
                }
                Iterator &operator =(const Iterator &) = default;

                Iterator &operator ++();
                Iterator &operator --();
                
                Font font;
                FontSet *set;
                size_t index;
            };
            static constexpr bool Supported = true;
        private:
            void *fc_pat;//< FcPattern
            void *fc_objset;//FcObjectSet
            void *fc_fontset;//FcFontSet
        #else
            //Unsupported
            struct Font{};
            struct Iterator;
            static constexpr bool Supported = false;
        #endif
        public:
            typedef Iterator iterator;
            
            //General methods
            ~FontSet();
            /**
             * @brief Locate Font in set
             * 
             * @param The font location
             * @return The font
             */
            Font operator [](size_t index) const;
            /**
             * @brief Get number of font 
             * 
             * @return The font count
             */
            size_t size() const;
            FontSet &operator =(FontSet &&);
            /**
             * @brief Get the first font
             * 
             * @return Iterator 
             */
            Iterator begin();
            /**
             * @brief Get the last font
             * 
             * @return Iterator 
             */
            Iterator end();
        public:
            //Internal function
            
        friend struct Iterator;
    };
    //TODO
    class BTKAPI FontMatcher{
        public:
            FontMatcher();
            FontMatcher(const FontMatcher &);
            ~FontMatcher();

            //Mask
            enum :Uint32{
                Filename,
                Fullname,
                Index,//<Uint32
                Style,//< String
                Language,//< String
            };
            void match();
            void get_value(Uint32 mask,void *p);
        private:
            //Impls
            #if BTK_X11
            void *pattern;
            #endif
    };
    class BTKAPI FontRenderer{

    };
    struct FontInfo{
        u8string fullname;
        u8string filename;
        Uint32 index;
    };
    /**
     * @brief Some useful function about font
     * 
     */
    namespace FontUtils{
        /**
         * @brief Get font's detail by it's name
         * 
         * @param name 
         * @return BTKAPI 
         */
        BTKAPI FontInfo FindFont(u8string_view name);
        /**
         * @brief Get the count of the font's faces
         * 
         * @param filename The font filename
         * @return BTKAPI 
         */
        BTKAPI size_t GetNumFace(u8string_view filename);
        BTKAPI u8string FindByCodepoint();
        /**
         * @brief Get font file by its name
         * 
         * @param name font name
         * @return std::string 
         */
        BTKAPI u8string GetFileByName(u8string_view name);
        /**
         * @brief Get the File By font face name
         * 
         * @param name font name
         * @return utf16 encoded filename string
         */
        BTKAPI u16string GetFileByName(u16string_view name);
        /**
         * @brief Init font utils
         * 
         */
        BTKAPI void Init();
        /**
         * @brief Quit font utils
         * 
         */
        BTKAPI void Quit();

        BTKAPI FontSet GetFontList();
        /**
         * @brief Get the Default Font filename
         * 
         * @return Utf encoded font name
         */
        BTKAPI u8string GetDefaultFont();
    };
    BTKAPI void AddMemFont(u8string_view name,const void *buf,size_t bufsize,bool dup = true);
    BTK_FLAGS_OPERATOR(FontStyle,int);
};


#endif // _BOX_FONT_HPP_
