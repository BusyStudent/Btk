#if !defined(_BTK_THEME_HPP_)
#define _BTK_THEME_HPP_
#include "pixels.hpp"
#include "defs.hpp"
//A file of themes
namespace Btk{
    struct Theme{
        //Window Background Color
        Color window_bg;
        //Text Color
        Color text_color;
    };
    namespace Themes{
        void   SetDefault(Theme *);
        Theme *GetDefault();
    };
};


#endif // _BTK_THEME_HPP_
