#if !defined(_BTKIMPL_FONT_HPP_)
#define _BTKIMPL_FONT_HPP_
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_pixels.h>
#include <string>
#include <string_view>
#include "atomic.hpp"
namespace Btk{
    struct FontImpl{
        //OpenFont
        FontImpl(std::string_view fname,int size);
        ~FontImpl();
        int ptsize;

        std::string filename;

        TTF_Font *font;
        Atomic refcount;
    };
};

#endif // _BTKIMPL_FONT_HPP_
