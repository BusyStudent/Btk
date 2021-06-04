#include "../build.hpp"

#include <Btk/font/system.hpp>
#include <Btk/font/font.hpp>
#include <Btk/exception.hpp>
#include <cstring>
#include <new>
namespace Btk::Ft{
    FT_Library Ft2Library = nullptr;
    Font::Font(const char *filename,Uint32 idx){
        GlobalCache().load_face(face,filename,idx);
    }
    Font::Font(const Font &font){
        face = font.face;
        ptsize = font.ptsize;
    }
    Font::~Font(){
        bitmap_free();
    }
    void Font::bitmap_free(){
        SDL_free(bitmap.buffer);
    }
    void Font::bitmap_realloc(int w,int h){
        if(w <= bitmap.w and h <= bitmap.h){
            //We just set the new w and h
            bitmap.w = w;
            bitmap.h = h;
            bitmap.pitch = sizeof(char) * w;
            return;
        }
        size_t new_size = w * h;
        Uint8 *buf = (Uint8*)SDL_realloc(bitmap.buffer,new_size);
        if(buf == nullptr){
            throw std::bad_alloc();
        }
        bitmap.buffer = buf;
        bitmap.w = w;
        bitmap.h = h;
        bitmap.pitch = sizeof(char) * w;
    }
    bool Font::bitmap_render(CharIndex idx){
        FT_Error err;
        err = FT_Set_Pixel_Sizes(face,0,ptsize);
        if(err){
            return false;
        }
        err = FT_Load_Glyph(face,idx,FT_LOAD_RENDER);
        if(err){
            return false;
        }
        int w,h,pitch;
        FT_GlyphSlot slot = face->glyph;
        w = slot->bitmap.width;
        h = slot->bitmap.rows;
        pitch = slot->bitmap.pitch;

        bitmap_realloc(w,h);
        BTK_ASSERT(slot->bitmap.pitch == w);
        //Copy it
        memcpy(bitmap.buffer,slot->bitmap.buffer,pitch * h);
        return true;
    }
    int Font::kerning_size(CharIndex prev,CharIndex cur){
        FT_Vector vec;
        FT_Get_Kerning(face, prev, cur, FT_KERNING_DEFAULT, &vec);
        return vec.x >> 6;
    }
    int Font::advance(CharIndex idx){
        FT_Fixed adv;
        FT_Get_Advance(face,idx,FT_LOAD_NO_SCALE,&adv);
        return adv;
    }
    CharIndex Font::index_char(Char ch){
        return FT_Get_Char_Index(face,ch);
    }
}
namespace Btk::Ft{
    /**
     * @brief Config charmap here
     * 
     * @param face 
     */
    static void config_charmap(FT_Face face){
    //SDL_ttf code here
    /* Set charmap for loaded font */
    FT_CharMap found = nullptr;
    #ifndef NDEBUG /* Font debug code */
        for (int i = 0; i < face->num_charmaps; i++) {
            FT_CharMap charmap = face->charmaps[i];
            BTK_LOGINFO("Found charmap: platform id %d, encoding id %d", charmap->platform_id, charmap->encoding_id);
        }
    #endif
        if (!found) {
            for (int i = 0; i < face->num_charmaps; i++) {
                FT_CharMap charmap = face->charmaps[i];
                if (charmap->platform_id == 3 && charmap->encoding_id == 10) { /* UCS-4 Unicode */
                    found = charmap;
                    break;
                }
            }
        }
        if (!found) {
            for (int i = 0; i < face->num_charmaps; i++) {
                FT_CharMap charmap = face->charmaps[i];
                if ((charmap->platform_id == 3 && charmap->encoding_id == 1) /* Windows Unicode */
                || (charmap->platform_id == 3 && charmap->encoding_id == 0) /* Windows Symbol */
                || (charmap->platform_id == 2 && charmap->encoding_id == 1) /* ISO Unicode */
                || (charmap->platform_id == 0)) { /* Apple Unicode */
                    found = charmap;
                    break;
                }
            }
        }
        if (found) {
            /* If this fails, continue using the default charmap */
            FT_Set_Charmap(face, found);
        }
    }
    Ft2Face::Ft2Face(const char *fname,Uint32 idx){
        FT_Error err = FT_New_Face(
            Ft2Library,
            fname,
            idx,
            &face
        );
        if(err){
            face = nullptr;
            throwRuntimeError("Could not new mem face");
        }
        config_charmap(face);
    }
    Ft2Face::Ft2Face(FontBuffer buf,Uint32 idx){
        buffer = buf;
        FT_Error err = FT_New_Memory_Face(
            Ft2Library,
            buffer.data(),
            buffer.size(),
            idx,
            &face
        );
        if(err){
            face = nullptr;
            throwRuntimeError("Could not new mem face");
        }
        config_charmap(face);
    }
    Ft2Face::~Ft2Face(){
        FT_Done_Face(face);
    }
}
//Code from fontstash
//
// Copyright (c) 2009-2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//
namespace Btk::Ft{
    void Font::fs_get_vmetrics(int *ascent,int *descent,int *lineGap){
        *ascent = face->ascender;
        *descent = face->descender;
        *lineGap = face->height - (*ascent - *descent);
    }
    float Font::fs_get_pixel_height_scale(float size){
        return size / face->units_per_EM;
    }
    bool  Font::fs_build_glyph(int glyph, float size, float scale,
							  int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1){
        set_ptsize(size);
        if(FT_Set_Pixel_Sizes(face,0,size)){
            return false;
        }
        if(FT_Load_Glyph(face,glyph,FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT)){
            return false;
        }
        FT_GlyphSlot ftGlyph = face->glyph;
        *advance = this->advance(glyph);
        *lsb = (int)ftGlyph->metrics.horiBearingX;
        *x0 = ftGlyph->bitmap_left;
        *x1 = *x0 + ftGlyph->bitmap.width;
        *y0 = -ftGlyph->bitmap_top;
        *y1 = *y0 + ftGlyph->bitmap.rows;
        return true;
    }
    void  Font::fs_render_glyph(unsigned char *output, int outWidth, int outHeight, int outStride,
								float scaleX, float scaleY, int glyph){
        int ftGlyphOffset = 0;
        unsigned int x, y;
        FT_GlyphSlot ftGlyph = face->glyph;
        // FONS_NOTUSED(outWidth);
        // FONS_NOTUSED(outHeight);
        // FONS_NOTUSED(scaleX);
        // FONS_NOTUSED(scaleY);
        //FONS_NOTUSED(glyph);	// glyph has already been loaded by fons__tt_buildGlyphBitmap
        FT_Set_Pixel_Sizes(face,0,ptsize);
        FT_Load_Glyph(face,glyph,FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT | FT_LOAD_RENDER);
        for ( y = 0; y < ftGlyph->bitmap.rows; y++ ) {
            for ( x = 0; x < ftGlyph->bitmap.width; x++ ) {
                output[(y * outStride) + x] = ftGlyph->bitmap.buffer[ftGlyphOffset++];
            }
        }
    }
}