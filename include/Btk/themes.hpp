#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include <iosfwd>
#include <map>

#include "string.hpp"
#include "pixels.hpp"
#include "font.hpp"
#include "defs.hpp"

//define theme context
#define BTK_THEME_PALETTE \
    BTK_THEME_FILED(Color,text)\
    BTK_THEME_FILED(Color,window)\
    BTK_THEME_FILED(Color,background)\
    BTK_THEME_FILED(Color,border)\
    BTK_THEME_FILED(Color,button)\
    BTK_THEME_FILED(Color,highlight)\
    BTK_THEME_FILED(Color,highlight_text)\


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
            Font font;
    };
    Theme *CurrentTheme();
    void   UseTheme(u8string_view name);
    void   AddTheme(u8string_view name,u8string_view config);
}

#endif // _BTK_THEME_HPP_
