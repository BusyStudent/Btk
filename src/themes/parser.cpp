//The theme parser
#ifndef BTK_NO_THEME_PARSER
#include "../build.hpp"
#include "../libs/ini.h"
#include <SDL2/SDL_rwops.h>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/themes.hpp>

#include <cstring>
namespace Btk{
namespace Themes{
    //Internal default theme
    Theme &DefaultTheme();
}
}


namespace{
    using Btk::Theme;
    using Btk::Color;
    using Btk::ParseHex;
    using Btk::ParseInt;
    //The context of parser
    struct ParseContext{
        Theme &theme;
        std::string fontname;
        int font_ptsize = 12;
        bool err = false;
    };
    bool parse_color(std::string_view str,Color &color){
        Color c;//tmp color
        size_t beg = str.find('(');
        size_t end = str.find_last_of(')');
        size_t split;//The postion of ,
        //(R,G,B,A)
        
        if(beg == str.npos or end == str.npos){
            //We dnnot find it
            return false;
        }
        split = str.find(',',beg);
        if(split == str.npos){
            return false;
        }
        //Process R
        c.r = ParseInt(str.substr(beg + 1,split - beg - 1));
        //Process G
        size_t next = str.find(',',split + 1);
        if(next == str.npos){
            return false;
        }
        
        c.g = ParseInt(str.substr(split + 1,next - split - 1));
        split = next;
        //Process B
        next = str.find(',',split + 1);
        if(next == str.npos){
            return false;
        }
        
        c.b = ParseInt(str.substr(split + 1,next - split - 1));
        split = next;
        //Process A(default to 255)
        next = str.find(',',split + 1);
        if(next == str.npos){
            c.a = 255;
        }
        else{
            c.a = ParseInt(str.substr(split + 1,next - split - 1));
        }
        color = c;
        #ifndef NDEBUG
        fprintf(stderr,"    Generate new color{%d,%d,%d,%d}\n",
            int(color.r),
            int(color.g),
            int(color.b),
            int(color.a));
        #endif
        return true;
    }
    bool strcasecmp(const char *s1,const char *s2){
        #ifdef _WIN32
        return ::_stricmp(s1,s2) == 0;
        #else
        return ::strcasecmp(s1,s2) == 0;
        #endif
    }
    int process_ini(void *_ctxt,const char *section,const char *name,const char*value){
        
        auto &ctxt = *static_cast<ParseContext*>(_ctxt);
        #ifndef NDEBUG
        fprintf(stderr,"  section:%s name:%s value:%s\n",section,name,value);
        #endif
        if(strcasecmp(section,"Font")){
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
        else if(strcasecmp(section,"Color")){
            //Is setting color
            if(strcasecmp(name,"text")){
                if(not parse_color(value,ctxt.theme.text_color)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"background")){
                if(not parse_color(value,ctxt.theme.background_color)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"window")){
                if(not parse_color(value,ctxt.theme.window_color)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"button")){
                if(not parse_color(value,ctxt.theme.button_color)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"border")){
                if(not parse_color(value,ctxt.theme.border_color)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"high_light")){
                if(not parse_color(value,ctxt.theme.high_light)){
                    goto err;
                }
            }
            else if(strcasecmp(name,"high_light_text")){
                if(not parse_color(value,ctxt.theme.high_light_text)){
                    goto err;
                }
            }
            else{
                goto err;
            }
        }
        else{
            err:;
            ctxt.err = true;
            return 0;
        }
        return 1;
    }
}
namespace Btk{
    Theme Theme::Parse(std::string_view txt){
        Theme theme = Themes::DefaultTheme();

        ParseContext ctxt{theme};

        ini_parse_memory(txt.data(),txt.length(),process_ini,&ctxt);

        if(ctxt.err){
            throwRuntimeError("Failed to parse theme");
        }
        

        if(not ctxt.fontname.empty()){
            //We set the fontname
            theme.font.open(ctxt.fontname,ctxt.font_ptsize);
        }

        return theme;
    }
    Theme Theme::ParseFile(std::string_view txt){
        Theme theme = Themes::DefaultTheme();

        ParseContext ctxt{theme};

        if(ini_parse(txt.data(),process_ini,&ctxt) == -1){
            throwRuntimeError(cformat("fopen('%s') failed",txt.data()).c_str());
        }

        if(ctxt.err){
            throwRuntimeError("Failed to parse theme");
        }
        

        if(not ctxt.fontname.empty()){
            //We set the fontname
            theme.font.open(ctxt.fontname,ctxt.font_ptsize);
        }

        return theme;
    }
}

#endif