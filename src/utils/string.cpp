#include "../build.hpp"
#include <SDL2/SDL_stdinc.h>
#include <Btk/utils/string.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/exception.hpp>
#include <cctype>
namespace Btk{
    String String::trim() const{
        const_iterator ibeg = begin();
        const_iterator iend = end();
        while(ibeg != iend){
            if(isspace(*ibeg)){
                ++ibeg;
            }
            else{
                break;
            }
        }
        while(ibeg != iend){
            if(isspace(*iend)){
                --iend;
            }
            else{
                break;
            }
        }
        return {ibeg,iend};
    }

    std::string String::encode(const char *to) const{
        char *result = SDL_iconv_string(
            to,
            "UTF16",
            reinterpret_cast<const char*>(str.data()),
            (str.length() + 1) * sizeof(char16_t));
        if(result == nullptr){
            throwSDLError();
        }
        std::string ret(result);
        SDL_free(result);
        return ret;
    }
}
#ifdef _WIN32
//Win32 impl
#include <windows.h>
#include <cwchar>
#include <cstring>
namespace Btk{
    String String::Format(const char16_t *fmt,...){
        int strsize;

        //Get the size of the string
        va_list varg;
        va_start(varg,fmt);
        strsize = _vscwprintf(reinterpret_cast<const wchar_t*>(fmt),varg);
        va_end(varg);
        
        String str;
        str->resize(strsize);

        //start formatting
        va_start(varg,fmt);
        vswprintf(
            reinterpret_cast<wchar_t*>(&str[0]),
            reinterpret_cast<const wchar_t*>(fmt),
            varg);
        
        va_end(varg);

        return str;
    }
}
#endif

#ifdef __linux
namespace{
    using Btk::Impl::VaListGuard;
    void print_int(std::u16string &out,int number){
        //todo ...
        int n = number % 10;
        while(n){

        }
    }
    /**
     * @brief process ctyle format in utf16
     * 
     * @param out 
     * @param fmt 
     * @param varg 
     */
    void u16sprintf(std::u16string &out,std::u16string_view fmt,va_list varg){
        size_t prev = 0;// prev position
        size_t cur = fmt.find(u'%');//current position
        while(cur != fmt.npos){
            //process formattor
            switch(fmt[cur + 1]){
                case u'%':{
                    out += u'%';
                    break;
                }
                case u'd':{
                    //%d
                    print_int(out,va_arg(varg,int));
                    break;
                }
                case u's':{
                    //%s
                    out += va_arg(varg,const char16_t*);
                    break;
                }
                default:{
                    //what should i do
                }
            }
            prev = cur + 1;
            cur = fmt.find(u'%',prev);
        }
        out += fmt.substr(prev,cur - 1);
    }
}

#endif