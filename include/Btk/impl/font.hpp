#if !defined(_BTKIMPL_FONT_HPP_)
#define _BTKIMPL_FONT_HPP_
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_pixels.h>
#include <string>
#include <string_view>
#include "atomic.hpp"
namespace Btk{
    /**
     * @brief Font Impl
     * 
     */
    struct FontImpl{
        FontImpl(std::string_view fname,int size);
        ~FontImpl();
        //ref Font
        FontImpl *ref();
        void unref();
        
        int ptsize;
        std::string filename;

        //TTF_Font *font;
        Atomic refcount;
    };
};

#endif // _BTKIMPL_FONT_HPP_
