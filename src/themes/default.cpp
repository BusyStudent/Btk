#include "../build.hpp"

#include <Btk/themes.hpp>
#include <atomic>
namespace Btk{
namespace Themes{
    static std::atomic<Theme*> current;
    //This is default themes color
    static Theme &default_theme(){
        static Theme theme = {
            //background_color
            {
                239,//R
                240,//G
                241,//B
                255//A
            },
            //text_color
            {
                0,
                0,
                0,
                255
            },
            {
               "NotoSansCJK",12
            },
            //border_color
            {
                208,
                208,
                208,
                255
            },
            //High Light color
            {
                61,
                174,
                233,
                255
            },
            //high_light_text
            {
                255,
                255,
                255,
                255
            }
        };
        return theme;
    }
    
    Theme &GetDefault(){
        Theme *theme = current.load(
            std::memory_order::memory_order_consume
        );
        if(theme == nullptr){
            return default_theme();
        }
        return *theme;
    }
    void  SetDefault(Theme *theme){
        if(theme == nullptr){
            theme = &default_theme();
        }
        current.store(
            theme,std::memory_order::memory_order_relaxed
        );
    }
};
};