#include "build.hpp"

#if 0
//Make sure it wonnot compile in release
//Still develop 

#if defined(BTK_USE_PCRE)
    #define BTK_PCRE_REGEX
    #include <pcreposix.h>
#elif defined(BTK_USE_STDREGEX)
    #define BTK_STD_REGEX
    #include <regex>
#elif defined(__linux__) && defined(__USE_GNU)
    //GNU Regex
    #define BTK_GNU_REGEX
    #include <regex.h>
#else
    //Using std regex
    #define BTK_STD_REGEX
    #include <regex>
#endif

//Utils Macro for translate flags
#define BTK_REGEX_ONFLAGS(V,F) \
    if((V & Btk::Regex::F) == Btk::Regex::F)

//GNU Begin
#ifdef  BTK_GNU_REGEX
#define BTK_REGEX_IMPL   ::GnuRegex
#define BTK_REGEX_RESULT ::GnuResult

//Forward
namespace{
    struct GnuRegex;
    struct GnuResult;
}
#include <Btk/regex.hpp>
//Impl
namespace {
    using Btk::u8string_view;
    using Btk::u8string;
    using Btk::RegexError;
    using RegexFlags = Btk::Regex::Flags;

    struct GnuRegexErrorInfo{
        int errcode;
        regex_t *regex = nullptr;
        u8string_view exp;
    };
    [[noreturn]]
    static inline void throwRegexError(
        int errcode,
        regex_t *regex = nullptr,
        u8string_view exp = {}
    ){
        GnuRegexErrorInfo info = {
            errcode,
            regex,
            exp
        };
        throw RegexError(&info);
    }
    static inline int TranslateFlags(RegexFlags f){
        int v = 0;
        BTK_REGEX_ONFLAGS(f,Basic){
            //Ignore 
        }
        BTK_REGEX_ONFLAGS(f,Extended){
            v |= REG_EXTENDED;
        }
        BTK_REGEX_ONFLAGS(f,ICase){
            v |= REG_ICASE;
        }
        return v;
    }
    struct BTKHIDDEN GnuRegex:public regex_t{
        GnuRegex(u8string_view exp,RegexFlags f);
        GnuRegex(const GnuRegex &);
        ~GnuRegex();
        //The expression
        u8string expression;
        //< Is succeed compiled
        bool compiled = false;
        int  flags = 0;

        void clear();
    };
    GnuRegex::~GnuRegex(){
        clear();
    }
    GnuRegex::GnuRegex(u8string_view exp,RegexFlags f){
        int errcode;
        expression = exp;
        errcode = regcomp(this,expression.c_str(),0);
        compiled = (errcode == 0);
        if(not compiled){
            throwRegexError(errcode);
        }
    }
    void GnuRegex::clear(){
        if(compiled){
            regfree(this);
        }
    }
}
/**
 * @brief Construct a new Regex Error:: Regex Error object
 * 
 * @param private_ 
 */
Btk::RegexError::RegexError(void *private_){
    auto info = static_cast<GnuRegexErrorInfo*>(private_);
    //Get error len and write it 
    size_t bufsize = regerror(info->errcode,info->regex,nullptr,0);
    message().resize(bufsize);
    regerror(info->errcode,info->regex,message().data(),bufsize);
    //Set compile expression
    _expression = info->exp;
}
#endif
//GNU End

//StdBegin
#ifdef BTK_STD_REGEX
namespace{
    using Btk::u8string_view;
    using Btk::u8string;
    using Btk::RegexError;
    struct StdRegex{
        std::regex;
    };
}
#endif
//StdEnd

namespace Btk{
    Regex::~Regex(){
        delete regex;
    }
    //RegexError
    RegexError::~RegexError() = default;
}

#endif