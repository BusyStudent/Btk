#include "build.hpp"

#include <Btk/regex.hpp>

#if __has_include(<regex.h>) && !BTK_MINGW
// #if 0

#ifndef REG_NOERROR
    #define REG_NOERROR REG_OK
#endif

#include <regex.h>
namespace Btk{
    struct Regex::Impl:public regex_t{
        u8string expression;
    };
    void Regex::compile(u8string_view exp,Flags f){
        cleanup();

        regex = new Impl;
        regex->expression = exp;
        //Compile
        int flags = 0;
        if((f & Basic) == Basic){
            flags |= REG_EXTENDED;
        }
        if((f & ICase) == ICase){
            flags |= REG_ICASE;
        }
        int err = ::regcomp(regex,regex->expression.c_str(),flags);
        if(err != 0){
            RegexError exception;
            size_t bufsize = regerror(err,regex,nullptr,0);
            exception.message().resize(bufsize);
            regerror(err,regex,exception.message().data(),bufsize);
            exception._expression = regex->expression;
            //Cleanup regex
            delete regex;
            regex = nullptr;
            throw exception;
        }
        //Done
    }
    void Regex::cleanup(){
        if(regex != nullptr){
            regfree(regex);
            delete regex;
            regex = nullptr;
        }
    }
    auto Regex::expression() const -> u8string_view{
        return regex->expression;
    }
    auto Regex::match(const u8string &str,size_t max) -> StringList{
        StringList reflist;
        regmatch_t match;

        const char *current = str.c_str();

        while(::regexec(regex,current,1,&match,0) == REG_NOERROR){
            if(reflist.size() == max){
                break;
            }
            int len = match.rm_eo - match.rm_so;
            reflist.emplace_back(current + match.rm_so,len);
            current += match.rm_eo;
        }
        return reflist;
    }
    void Regex::replace(u8string &str,u8string_view to,size_t max){
        regmatch_t match;

        size_t n = 0;
        size_t pos = 0;

        const char *current = str.c_str();
        while(::regexec(regex,current,1,&match,0) == REG_NOERROR){
            if(n == max){
                break;
            }
            n += 1;

            int len = match.rm_eo - match.rm_so;
            pos = current - str.c_str();
            //Replace
            str.base().replace(match.rm_so + pos,len,to.base());
            //Move
            current = str.c_str() + pos + match.rm_so + to.size();
        }
    }
    RegexError::~RegexError() = default;
}

#elif 1

#include <cstdio>
#include <cctype>

#define RE_REALLOC SDL_realloc
#define RE_MALLOC  SDL_malloc
#define RE_FREE    SDL_free


#include "./libs/re.c"


namespace Btk{
    struct Regex::Impl:public re_pattern_t{
        u8string expression;
        re_t re;
    };
    void Regex::cleanup(){
        if(regex != nullptr){
            re_cleanup(regex);
            delete regex;
        }
    }
    void Regex::compile(u8string_view exp,Flags flags){
        if(flags != Flags::Basic){
            throwRuntimeError("Unsupported");
        }
        if(regex == nullptr){
            regex = new Impl;
            re_init(regex);
        }
        //Reset regex
        re_reset(regex);
        regex->expression = exp;
        
        re_t re;
        re = re_compile(regex->expression.c_str(),regex);

        if(re == nullptr){
            RegexError exception;
            exception.set_message("Bad regex");
            exception._expression = exp;
            throw exception;
        }
        regex->re = re;
        //done
        #ifndef NDEBUG
        re_print(regex);
        #endif
    }
    auto Regex::expression() const -> u8string_view{
        return regex->expression;
    }
    auto Regex::match(const u8string &input,size_t max) -> StringList{
        const char *current = input.c_str();
        int length;
        int ret;

        StringList reflist;
        ret = re_matchp(regex->re,current,&length);
        while(ret != -1){
            if(reflist.size() == max){
                break;
            }
            //Add
            reflist.emplace_back(current + ret,length);
            //Move
            current += (ret + length);
            //Match
            ret = re_matchp(regex->re,current,&length);
        }

        return reflist;
    }
    void Regex::replace(u8string &str,u8string_view to,size_t max){
        const char *current = str.c_str();
        size_t pos = 0;
        size_t n = 0;
        int length;
        int ret;

        ret = re_matchp(regex->re,current,&length);
        while(ret != -1){
            if(n == max){
                break;
            }
            n += 1;
            //Get position
            pos = current - str.c_str();
            //Replace
            str.base().replace(ret + pos,length,to.base());
            //Move
            current = str.c_str() + pos + ret + to.size();
            //Match
            ret = re_matchp(regex->re,current,&length);
        }
    }
    RegexError::~RegexError() = default;
}

#else

#include <regex>
//Use std regex
//FIXME STILL has bug?
namespace Btk{
    struct Regex::Impl{
        u8string expression;
        std::regex re;
    };
    void Regex::cleanup(){
        delete regex;
    }
    void Regex::compile(u8string_view exp,Flags f){
        //new
        if(regex == nullptr){
            regex = new Impl;
        }
        std::regex_constants::syntax_option_type opts = {};

        //Check flags
        if((f & Basic) == Basic){
            opts |= std::regex::extended;
        }
        if((f & ECMAScript) == ECMAScript){
            opts |= std::regex::ECMAScript;
        }
        if((f & ICase) == ICase){
            opts |= std::regex::icase;
        }

        try{
            //Build
            std::regex reg(exp.data(),exp.size(),opts);

            //Move
            regex->re = std::move(reg);
            regex->expression = exp;
        }
        catch(std::regex_error &err){
            RegexError exception;
            exception.set_message(err.what());
            exception._expression = exp;
            throw exception;
        }
    }
    //Match
    auto Regex::match(const u8string &str,size_t max) -> StringList{
        std::smatch m;
        
        if(std::regex_match(str.base(),m,regex->re)){
            StringList list;
            for(auto &each : m){
                list.emplace_back(each.str());
            }
            return list;
        }

        return {};
    }
    // void Regex::replace(u8string &str,u8string_view to,size_t max){
    //     std::regex_replace(str.base(),regex->re,to.base());
    // }
    RegexError::~RegexError() = default;
}



#endif