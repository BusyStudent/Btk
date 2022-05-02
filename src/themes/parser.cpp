//The theme parser
#ifndef BTK_NO_THEME_PARSER
#include "../build.hpp"
#include <SDL2/SDL_rwops.h>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/themes.hpp>
#include <cstring>

#include "../libs/ini.h"
#include "../libs/ini.c"

#define PARSER_BEGIN if(0){}
#define PARSER_ON_STI(name) else if(Btk::strcasecmp(name,section) == 0)
#define PARSER_ON_KEY(name) else if(Btk::strcasecmp(name,key) == 0)
#define PARSER_END else{}


namespace{
    using Btk::Theme;
    using Btk::Color;
    using Btk::ParseColor;
    using Btk::ParseHex;
    using Btk::ParseInt;
        
    int process_ini(void *_theme,const char *section,const char *key,const char *value){
        Theme *theme = static_cast<Theme*>(_theme);

        auto process_palette = [](Theme::Palette &p,const char *key,const char *value){
            PARSER_BEGIN

            #define BTK_THEME_FILED(TYPE,NAME) \
                PARSER_ON_KEY(#NAME){ \
                    p.NAME = ParseColor(value);\
                }

            BTK_THEME_PALETTE
            #undef BTK_THEME_FILED

            PARSER_END
        };

        PARSER_BEGIN

        PARSER_ON_STI("UI"){
            
        }
        PARSER_ON_STI("Font"){

        }
        PARSER_ON_STI("Palette::Active"){
            normal:
            process_palette(theme->active,key,value);
        }
        PARSER_ON_STI("Palette::Disabled"){
            disabled:
            process_palette(theme->disabled,key,value);
        }

        PARSER_END
        return 1;
    }
}
namespace Btk{
    Theme Theme::Parse(u8string_view txt){
        Theme theme;
        if(not ini_parse_memory(txt.data(),txt.length(),process_ini,&theme)){
            throwRuntimeError(SDL_GetError());
        }
        return theme;
    }
}

#endif