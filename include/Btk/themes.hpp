#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include <iosfwd>

#include "utils/template.hpp"
#include "graphics/base.hpp"
#include "string.hpp"
#include "pixels.hpp"
#include "font.hpp"
#include "defs.hpp"

//define theme context
#define BTK_THEME_PALETTE \
    BTK_THEME_FILED(Brush,text)\
    BTK_THEME_FILED(Brush,window)\
    BTK_THEME_FILED(Brush,background)\
    BTK_THEME_FILED(Brush,border)\
    BTK_THEME_FILED(Brush,button)\
    BTK_THEME_FILED(Brush,button_text)\
    BTK_THEME_FILED(Brush,highlight)\
    BTK_THEME_FILED(Brush,highlight_text)\
    BTK_THEME_FILED(Brush,placeholder_text)\

//Docs for Palette
//text => for Text and Button
//window => for Window background
//Background => for textbox background
//border => for border
//button => for button background
//highlight => for highlight background
//highlight_text => for highlight text color

namespace Btk{
    class Theme{
        public:
            /**
             * @brief Construct a new empty Theme object
             * 
             */
            Theme();
            Theme(const Theme &);
            ~Theme();
        public:
            //Parser
            static Theme Parse(u8string_view text);
        public:
            //All data
            #define BTK_THEME_FILED(TYPE,VAR) \
                TYPE VAR = {};
            struct Palette{
                BTK_THEME_PALETTE
            } active,inactive,disabled;
            #undef  BTK_THEME_FILED
            //Font
            Font font;
            //Button 
            float button_radius = 1;
            //Menu
            float menubar_height = 25;//???
            //Slider fixed size constant
            float slider_circle_radius = 10.0f;
            float slider_bar_radius = 4.0f;
            float slider_bar_width = 10.0f;
            float slider_bar_height = 10.0f;

    };
    /**
     * @brief Style for widget
     * 
     */
    class Style{
        //TODO 
    };
    RefPtr<Theme> CurrentTheme();
    /**
     * @brief Change current theme to
     * 
     * @param name 
     */
    void   UseTheme(u8string_view name);
    void   AddTheme(u8string_view name,u8string_view config);
}

#endif // _BTK_THEME_HPP_
