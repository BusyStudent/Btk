#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include <iosfwd>
#include <map>

#include "string.hpp"
#include "pixels.hpp"
#include "font.hpp"
#include "defs.hpp"
//A file of themes

//Macro to gen color constant

#ifdef BTK_VSCODE_SUPPRESS
    //For suppress the error
    #define BTK_MAKE_COLOR(COLOR) static Btk::u8string_view COLOR;
#else
    #define BTK_MAKE_COLOR(COLOR) \
        static constexpr Btk::u8string_view COLOR = #COLOR;
#endif
namespace Btk{
    class Palette;
    /**
     * @brief Class for set or get the color from it
     * 
     */
    struct _PaletteProxy{
        Palette *palette;
        u8string_view key;
        /**
         * @brief load the color
         * 
         * @return Color 
         */
        operator Color() const;
        void operator =(Color c) const;
    };

    /**
     * @brief Colors
     * 
     */
    class BTKAPI Palette{
        public:
            Palette();
            Palette(const Palette &);
            Palette(Palette &&);
            ~Palette();

            using Proxy = _PaletteProxy;


            void  set(u8string_view key,Color c);
            Color get(u8string_view key) const;

            Palette &operator =(Palette &&);
            Palette &operator =(const Palette &);
            /**
             * @brief Get the value from the Palette
             * 
             * @param key 
             * @return Proxy 
             */
            Proxy operator [](u8string_view key){
                return Proxy{this,key};
            }
            /**
             * @brief Get value form it
             * 
             * @param key 
             * @return Color 
             */
            Color operator [](u8string_view key) const{
                return get(key);
            }
            /**
             * @brief get the size of the 
             * 
             * @return size_t 
             */
            size_t size() const;
            bool has_color(u8string_view key) const;
        private:
            std::map<u8string,Color> colors;
        friend BTKAPI std::ostream &operator <<(std::ostream &,const Palette &);
    };

    inline _PaletteProxy::operator Color() const{
        return palette->get(key);
    }
    inline void _PaletteProxy::operator =(Color c) const{
        palette->set(key,c);
    }

    class BTKAPI Theme{
        public:
            Theme():ptr(nullptr){};
            Theme(const Theme &t);
            Theme(Theme &&);
            ~Theme();
            //const begin
            BTK_MAKE_COLOR(Window);//< for Window's Background
            BTK_MAKE_COLOR(Background); //< for Background
            BTK_MAKE_COLOR(Button); //< for Button
            BTK_MAKE_COLOR(Border); //< for Border

            BTK_MAKE_COLOR(Text); //< for Text
            BTK_MAKE_COLOR(HighlightedText); //< for HighlightedText
            BTK_MAKE_COLOR(Highlight); //< for Highlight
            //const end
            /**
             * @brief Generate by config
             * 
             * @param txt The config text
             * @return Theme 
             */
            static Theme Parse(u8string_view txt);
            static Theme ParseFile(u8string_view txt);

            Palette &normal(){
                return ptr->active;
            }
            Palette &disabled(){
                return ptr->disabled;
            }
            Palette &active(){
                return ptr->active;
            }
            Palette &inactive(){
                return ptr->inactive;
            }

            const Palette &normal() const{
                return ptr->active;
            }
            const Palette &disabled() const{
                return ptr->disabled;
            }
            const Palette &active() const{
                return ptr->active;
            }
            const Palette &inactive() const{
                return ptr->inactive;
            }
            /**
             * @brief Get color from it
             * 
             * @param v 
             * @return decltype(auto) 
             */
            decltype(auto) operator [](u8string_view v){
                return normal()[v];
            }
            Theme &operator =(const Theme &t);
            
            u8string_view font_name() const{
                return ptr->font;
            }
            float font_size() const{
                return ptr->ptsize;
            }
            void set_font_ptsize(float ptsize){
                ptr->ptsize = ptsize;
            }
            void set_font_name(u8string_view name){
                ptr->font = name;
            }
            /**
             * @brief Create an empty theme
             * 
             * @return Theme 
             */
            static Theme Create();
        private:
            struct Base{
                Palette disabled;
                Palette active;
                Palette inactive;
                //Font
                u8string font = "NotoSansCJK";
                float ptsize = 12;

                int refcount = 1;

                Base *ref(){
                    ++refcount;
                    return this;
                }
                void unref(){
                    --refcount;
                    if(refcount <= 0){
                        delete this;
                    }
                }
            };
            Theme(Base *b):ptr(b){}
            Base *ptr;
    };
    inline Theme::Theme(const Theme &t){
        if(t.ptr == nullptr){
            ptr = nullptr;
            return;
        }
        ptr = t.ptr->ref();
    }
    inline Theme::Theme(Theme &&t){
        ptr = t.ptr;
        t.ptr = nullptr;
    }

    namespace Themes{
        BTKAPI Theme &GetDefault();
    };
    BTKAPI std::ostream &operator <<(std::ostream &,const Palette &);
}

#undef BTK_MAKE_COLOR

#endif // _BTK_THEME_HPP_
