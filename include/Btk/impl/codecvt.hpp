#if !defined(_BTKIMPL_CODECVT_HPP_)
#define _BTKIMPL_CODECVT_HPP_
#include <SDL2/SDL_stdinc.h>
#include <string>
#include <string_view>
#include "../exception.hpp"
namespace Btk{
    inline std::u16string Utf8ToUtf16(std::string_view utf8){
        char *str = SDL_iconv_string("UTF16","UTF8",utf8.data(),utf8.size());
        if(str == nullptr){
            throwSDLError();
        }
        std::u16string utf16(reinterpret_cast<char16_t*>(str));
        SDL_free(str);
        return utf16;
    }
};
#endif // _BTKIMPL_CODECVT_HPP_
