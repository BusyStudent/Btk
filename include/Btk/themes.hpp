#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include "pixels.hpp"
#include "defs.hpp"
//A file of themes
namespace Btk{
    struct Theme{
        //Background Color
        Color background_color;
        //Text Color
        Color text_color;
        //Default font name
        const char *font;
        //Default font size
        int         font_ptsize;
        //Border Color
        Color border_color;
        //Hight light Color
        Color high_light;
        //Hight light Text
        Color high_light_text;
    };
    namespace Themes{
        BTKAPI void   SetDefault(Theme *);
        BTKAPI Theme &GetDefault();
    };
};


#endif // _BTK_THEME_HPP_
