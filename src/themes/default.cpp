#include "../build.hpp"

#include <Btk/detail/thread.hpp>
#include <Btk/themes.hpp>
#include <Btk/Btk.hpp>
#include <atomic>
#include <mutex>

#include "builtins.inl"

namespace Btk{
// namespace Themes{
//     static SpinLock spinlock;
//     //This is default themes color
//     Theme &DefaultTheme(){
//         static Theme theme = {
//             //Window background
//             {
//                 239,//R
//                 240,//G
//                 241,//B
//                 255//A
//             },
//             //background_color
//             {
//                 255,//R
//                 255,//G
//                 255,//B
//                 255//A
//             },
//             //text_color
//             {
//                 0,
//                 0,
//                 0,
//                 255
//             },
//             {
//                "NotoSansCJK",12
//             },
//             //border_color
//             {
//                 208,
//                 208,
//                 208,
//                 255
//             },
//             //High Light color
//             {
//                 61,
//                 174,
//                 233,
//                 255
//             },
//             //high_light_text
//             {
//                 255,
//                 255,
//                 255,
//                 255
//             },
//             //Button color
//             {
//                 233,
//                 234,
//                 235,
//                 255
//             }
//         };
//         return theme;
//     }
    
//     Theme GetDefault(){
//         std::lock_guard locker(spinlock);
//         #ifndef BTK_NO_THEME_PARSER
//         //Get theme file from user
//         char *conf = getenv("BTK_THEME");
//         if(conf != nullptr){
//             try{
//                 BTK_LOGINFO("Try parsing theme conf => %s",conf);
//                 return Theme::ParseFile(conf);
//             }
//             catch(...){
//                 //error
//                 //use default theme
//             }
//         }
//         #endif
//         return DefaultTheme();
//     }
//     void  SetDefault(const Theme &theme){
//         std::lock_guard locker(spinlock);
//         DefaultTheme() = theme;
//     }
// };
    Theme::Theme(){

    }
    Theme::Theme(const Theme &) = default;
    Theme::~Theme() = default;

    static bool has_theme_inited = false;
    static Constructable<RefPtr<Theme>> global_theme;

    static void theme_cleanup(){
        global_theme.destroy();
    }

    RefPtr<Theme> CurrentTheme(){
        if(not has_theme_inited){
            global_theme.construct(new Theme);
            construct_theme_default(**global_theme);
            has_theme_inited = true;
            AtExit(theme_cleanup);
        }
        return *global_theme;
    }
}