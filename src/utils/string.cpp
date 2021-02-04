#include "../build.hpp"
#include <SDL2/SDL_stdinc.h>
#include <Btk/utils/string.hpp>
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