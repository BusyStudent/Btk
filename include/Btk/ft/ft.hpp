#if !defined(_BTK_FT_HPP_)
#define _BTK_FT_HPP_
//Freetype impl files
#include "../defs.hpp"
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_ERRORS_H
#include FT_TRUETYPE_IDS_H
struct SDL_RWops;
namespace BtkFt{
    struct BTKAPI Face{
        ~Face();
        FT_Face face = nullptr;
    };
}

#endif // _BTK_FT_HPP_
