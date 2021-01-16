#if !defined(_BOX_FONT_HPP_)
#define _BOX_FONT_HPP_
#include <cstddef>
#include <string>
#include <string_view>
#include "pixels.hpp"
#include "defs.hpp"
namespace Btk{
    struct Size;
    struct FontImpl;
    /**
     * @brief FontStyle from SDL_ttf
     * 
     */
    enum class FontStyle:int{
        Normal = 0x00,
        Bold = 0x01,
        Italic = 0x02,
        Underline = 0x04,
        Strikethrough = 0x08
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
            Font(std::string_view fontname,int ptsize);
            ~Font();
            /**
             * @brief Render solid text
             * 
             * @note This one is fastest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_solid(std::string_view text,Color color);
            PixBuf render_solid(std::u16string_view text,Color color);
            /**
             * @brief Render shaded text
             * 
             * @note This one is middle speed
             * @param text Your text
             * @param fg Text color
             * @param bg BackGround color
             * @return PixBuf 
             */
            PixBuf render_shaded(std::string_view text,Color fg,Color bg);
            PixBuf render_shaded(std::u16string_view text,Color fg,Color bg);
            /**
             * @brief Render blended text
             * 
             * @note This one is slowest
             * @param text Your text
             * @param color Text color
             * @return PixBuf 
             */
            PixBuf render_blended(std::string_view text,Color color);
            PixBuf render_blended(std::u16string_view text,Color color);
            /**
             * @brief Get Kerning Size in two chars
             * 
             * @param prev_ch The first char
             * @param ch The next char
             * @return The kerning
             */
            int kerning_size(char16_t prev_ch,char16_t ch) const;
            /**
             * @brief Check the font has the glyph
             * 
             * @param ch The utf16 char
             * @return true 
             * @return false 
             */
            bool has_glyph(char16_t ch) const;
            /**
             * @brief Get font height
             * 
             * @return int 
             */
            int height() const;
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
            Size size(std::string_view text);
            Size size(std::u16string_view text);
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
            void open(std::string_view fontname,int ptsize);
            /**
             * @brief Open font by its filename
             * 
             * @param filename Font filename
             * @param ptsize Font ptsize
             */
            void openfile(std::string_view filename,int ptsize);
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
            FontImpl *impl() const noexcept{
                return pimpl;
            }
            /**
             * @brief Get font family name
             * 
             * @return family name
             */
            std::string family() const;
            std::string style_name() const;
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
            static Font FromFile(std::string_view filename,int ptsize);
        private:
            Font(FontImpl *i):pimpl(i){};
            FontImpl *pimpl;
    };
    class FontSet{
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
            struct Font{
                /**
                 * @note The return of the methods are all reference,
                 *       If you want to use it after destroying the fontset,
                 *       Please make a copy of the string_view
                 */
                std::string_view family() const;
                std::string_view style() const;
                std::string_view file() const;
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
        BTKAPI std::string GetFileByName(std::string_view name);
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
    };
    //operators for FontStyle
    inline FontStyle operator |(FontStyle s1,FontStyle s2) noexcept{
        return static_cast<FontStyle>(int(s1) | int(s2));
    }
    inline FontStyle operator +(FontStyle s1,FontStyle s2) noexcept{
        return static_cast<FontStyle>(int(s1) | int(s2));
    }
    inline FontStyle operator &(FontStyle s1,FontStyle s2) noexcept{
        return static_cast<FontStyle>(int(s1) & int(s2));
    }
    inline FontStyle operator +=(FontStyle s1,FontStyle s2) noexcept{
        return static_cast<FontStyle>(int(s1) | int(s2));
    }
    inline FontStyle operator |=(FontStyle s1,FontStyle s2) noexcept{
        return static_cast<FontStyle>(int(s1) | int(s2));
    }
};


#endif // _BOX_FONT_HPP_
