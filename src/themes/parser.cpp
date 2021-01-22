//The theme parser
#ifndef BTK_NO_THEME_PARSER
#include "../build.hpp"
#include "../libs/ini.h"
#include <SDL2/SDL_rwops.h>
#include <Btk/utils/mem.hpp>
#include <Btk/themes.hpp>

#include <cstring>

namespace{
    using Btk::Theme;
    using Btk::Color;
    using Btk::ParseHex;
    using Btk::ParseInt;
    //The context of parser
    struct ParseContext{
        Theme &theme;
        std::string fontname;
        int font_ptsize;
    };
    bool parse_color(std::string_view str,Color &color){
        if(*str.begin() == '(' and *str.end() == ')'){
            //Format like(R,G,B,A)
            //Or(R,G,B)
            size_t beg = str.find(',');
        }
    }
    bool strcasecmp(const char *s1,const char *s2){
        #ifdef _WIN32
        return ::_stricmp(s1,s2) == 0;
        #else
        return ::strcasecmp(s1,s2) == 0;
        #endif
    }
    int process_ini(void *_ctxt,const char *selection,const char *name,const char*value){
        auto &ctxt = *static_cast<ParseContext*>(_ctxt);
        if(strcasecmp(selection,"Font")){
            //Font name?
            if(strcasecmp(name,"name")){
                ctxt.fontname = value;
            }
            //FontPtsize
            else if(strcasecmp(name,"ptsize")){
                ctxt.font_ptsize = std::atoi(value);
            }
            else{
                //Unknown value
                goto err;
            }
        }
        else if(strcasecmp(selection,"Color")){
            //Is setting color
            if(strcasecmp(name,"text")){
                parse_color(value,ctxt.theme.text_color);
            }
            else if(strcasecmp(name,"background")){
                parse_color(value,ctxt.theme.border_color);
            }
            else if(strcasecmp(name,"border")){
                parse_color(value,ctxt.theme.border_color);
            }
            else if(strcasecmp(name,"high_light")){
                parse_color(value,ctxt.theme.high_light);
            }
            else if(strcasecmp(name,"high_light_text")){
                parse_color(value,ctxt.theme.high_light_text);
            }
            else{
                goto err;
            }
        }
        else{
            err:
            #ifndef NDEBUG
            fprintf(stderr,"  Unknown selection:%s %s=%s",selection,name,value);
            #endif
        }
        return 0;
    }
}
namespace Btk{
    Theme Theme::Parse(std::string_view txt){
        Theme theme = Themes::GetDefault();

        ParseContext ctxt = {theme};

        ini_parse_memory(txt.data(),txt.length(),process_ini,&ctxt);

        if(not ctxt.fontname.empty()){
            //We set the fontname
            theme.font.open(ctxt.fontname,ctxt.font_ptsize);
        }

        return theme;
    }
}

#endif