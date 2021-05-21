//The theme parser
#ifndef BTK_NO_THEME_PARSER
#include "../build.hpp"
#include "../libs/ini.h"
#include <SDL2/SDL_rwops.h>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include "../build.hpp"
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
    };
    //TODO Rewrite the color parser
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
        if(strcasecmp(section,"Color")){
            Color c;
            parse_color(value,c);
            ctxt.theme[name] = c;
        }
        else if(strcasecmp(section,"Font")){
            if(strcasecmp(name,"name")){
                ctxt.theme.set_font_name(value);
            }
            else if(strcasecmp(name,"ptsize")){
                ctxt.theme.set_font_ptsize(ParseInt(value));
            }
        }
        return 1;
    }
}
namespace Btk{
    Theme Theme::Parse(u8string_view txt){
        Theme theme = Theme::Create();

        ParseContext ctxt{theme};

        ini_parse_memory(txt.data(),txt.length(),process_ini,&ctxt);
        
        
        return theme;
    }
    Theme Theme::ParseFile(u8string_view txt){
        Theme theme = Theme::Create();

        ParseContext ctxt{theme};

        if(ini_parse(txt.data(),process_ini,&ctxt) == -1){
            throwRuntimeError(cformat("fopen('%s') failed",txt.data()).c_str());
        }
        return theme;
    }
}

#endif