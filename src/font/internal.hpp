#if !defined(_BTK_FONT_INTERNAL_HPP_)
#define _BTK_FONT_INTERNAL_HPP_
#include <Btk/impl/thread.hpp>
#include <Btk/defs.hpp>
#include <map>
#ifdef BTK_USE_FREETYPE
#include <ft2build.h>
#else
#include "../libs/stb_truetype.h"
#endif

namespace BTKHIDDEN BtkFt{
    struct GlyphSlot{
        
    };
    struct FontFace{
        
    };
    struct FontCache{

    };

    extern FontCache *GlobalCache;
}

#endif // _BTK_FONT_INTERNAL_HPP_
