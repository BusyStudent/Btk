#include "../build.hpp"

#include <Btk/font/font.hpp>
#include <Btk/exception.hpp>
#include <cstring>
#include <new>
namespace Btk::Ft{
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
    void Font::bitmap_render(CharIndex idx){
        FT_Error err;
        err = FT_Set_Pixel_Sizes(face,0,ptsize);
        err = FT_Load_Glyph(face,idx,FT_LOAD_RENDER);
        int w,h,pitch;
        FT_GlyphSlot slot = face->glyph;
        w = slot->bitmap.width;
        h = slot->bitmap.rows;
        pitch = slot->bitmap.pitch;

        bitmap_realloc(w,h);
        BTK_ASSERT(slot->bitmap.pitch == w);
        //Copy it
        memcpy(bitmap.buffer,slot->bitmap.buffer,pitch * h);
    }
    CharIndex Font::index_char(Char ch){
        return FT_Get_Char_Index(face,ch);
    }
}
namespace Btk::Ft{
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
    
}