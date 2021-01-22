#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include <string_view>
#include "pixels.hpp"
#include "font.hpp"
#include "defs.hpp"
//A file of themes
namespace Btk{
    struct BTKAPI Theme{
        //Background Color
        Color background_color;
        //Text Color
        Color text_color;
        //Default font
        Font  font;
        //Border Color
        Color border_color;
        //Hight light Color
        Color high_light;
        //Hight light Text
        Color high_light_text;
        /**
         * @brief Generate by config
         * 
         * @param txt The config text
         * @return Theme 
         */
        static Theme Parse(std::string_view txt);
    };
    namespace Themes{
        BTKAPI void   SetDefault(Theme *);
        BTKAPI Theme &GetDefault();
    };
};


#endif // _BTK_THEME_HPP_
