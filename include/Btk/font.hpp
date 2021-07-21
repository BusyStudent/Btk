#if !defined(_BOX_FONT_HPP_)
#define _BOX_FONT_HPP_
#include <cstddef>
#include "string.hpp"
#include "pixels.hpp"
#include "defs.hpp"
namespace Btk{
    namespace Ft{
        /**
         * @brief Font impl
         * 
         */
        struct Font;
    }
    struct Size;
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
    /**
     * @brief Font Class
     * 
     */
    class BTKAPI Font{
        public:
            //empty font
            Font():pimpl(nullptr){};
            Font(const Font &font);
            Font(Font &&f){
                pimpl = f.pimpl;
                f.pimpl = nullptr;
            };
            /**
             * @brief Construct a new Font object
             * 
             * @param fontname Font name
             * @param ptsize Font ptsize
             */
            Font(u8string_view fontname,int ptsize);
            ~Font();
            /**
             * @brief Render solid text
             * 
             * @note This one is fastest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_solid(u8string_view text,Color color);
            PixBuf render_solid(u16string_view text,Color color);
            /**
             * @brief Render shaded text
             * 
             * @note This one is middle speed
             * @param text Your text
             * @param fg Text color
             * @param bg BackGround color
             * @return PixBuf 
             */
            PixBuf render_shaded(u8string_view text,Color fg,Color bg);
            PixBuf render_shaded(u16string_view text,Color fg,Color bg);
            /**
             * @brief Render blended text
             * 
             * @note This one is slowest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_blended(u8string_view text,Color color);
            PixBuf render_blended(u16string_view text,Color color);
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
            int  ptsize() const noexcept;
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
            void set_ptsize(int new_ptsize);
            /**
             * @brief Get The text's w and h
             * 
             * @param text The text
             * @return w and h (-1 if failed)
             */
            Size size(u8string_view text);
            Size size(u16string_view text);
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
             */
            void open(u8string_view fontname,int ptsize);
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             */
            void openfile(u8string_view filename,int ptsize);
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
            /**
             * @brief Assign font
             * 
             * @return *this 
             */
            Font &operator =(const Font &);
            Font &operator =(Font &&);
            bool empty() const noexcept{
                return pimpl == nullptr;
            };
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             * @return Font 
             */
            static Font FromFile(u8string_view filename,int ptsize);
        private:
            Font(Ft::Font *i):pimpl(i){};
            Ft::Font *pimpl;
        friend class Renderer;
    };
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
        friend struct Iterator;
    };
    class BTKAPI FontMatcher{
        public:
            FontMatcher();
            FontMatcher(const FontMatcher &) = delete;
            ~FontMatcher();
        private:
            //Impls
            #ifdef __gnu_linux__
            void *pat;
            #endif
    };
    /**
     * @brief Some useful function about font
     * 
     */
    namespace FontUtils{
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
    //operators for FontStyle
    // inline FontStyle operator |(FontStyle s1,FontStyle s2) noexcept{
    //     return static_cast<FontStyle>(int(s1) | int(s2));
    // }
    // inline FontStyle operator +(FontStyle s1,FontStyle s2) noexcept{
    //     return static_cast<FontStyle>(int(s1) | int(s2));
    // }
    // inline FontStyle operator &(FontStyle s1,FontStyle s2) noexcept{
    //     return static_cast<FontStyle>(int(s1) & int(s2));
    // }
    // inline FontStyle operator +=(FontStyle s1,FontStyle s2) noexcept{
    //     return static_cast<FontStyle>(int(s1) | int(s2));
    // }
    // inline FontStyle operator |=(FontStyle s1,FontStyle s2) noexcept{
    //     return static_cast<FontStyle>(int(s1) | int(s2));
    // }
    BTK_FLAGS_OPERATOR(FontStyle,int);
};


#endif // _BOX_FONT_HPP_
