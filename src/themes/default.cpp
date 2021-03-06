#include "../build.hpp"

#include <Btk/impl/thread.hpp>
#include <Btk/themes.hpp>
#include <atomic>
#include <mutex>
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
    Theme::~Theme(){
        if(ptr != nullptr){
            ptr->unref();
        }
    }
    Theme Theme::Create(){
        return new Base;
    }
    Theme &Theme::operator =(const Theme &t){
        if(this != &t){
            if(ptr != nullptr){
                ptr->unref();
            }
            if(t.ptr != nullptr){
                ptr = t.ptr->ref();
            }
            else{
                ptr = nullptr;
            }
        }
        return *this;
    }
}
namespace Btk::Themes{
    Theme &GetDefault(){
        static auto t = Theme::Create();
        //Normal color
        t.normal()[Theme::Window] = Color(239,240,241,255);
        t.normal()[Theme::Background] = Color(255,255,255,255);
        t.normal()[Theme::Text] = Color(0,0,0,255);
        t.normal()[Theme::Border] = Color(208,208,208,255);
        t.normal()[Theme::Button] = Color(233,234,235,255);
        t.normal()[Theme::Highlight] = Color(61,174,233,255);
        t.normal()[Theme::HighlightedText] = Color(255,255,255,255);
        
        return t;
    }
}