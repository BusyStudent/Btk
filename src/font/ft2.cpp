#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_ERRORS_H
#include FT_GLYPH_H

#include <Btk/thirdparty/utf8.h>
#include <Btk/exception.hpp>

#include <cmath>
#include <algorithm>

#include "../build.hpp"
#include "internal.hpp"

namespace BtkFt{
    using Btk::throwRuntimeError;
    Library::Library(){
        ft_lib = nullptr;
        FT_Error err = FT_Init_FreeType(&ft_lib);
        if(err){
            throwRuntimeError("Init freetype failed");
        }
    }
    Library::~Library(){
        for(auto &iter:faces_map){
            delete iter.second;
        }
        FT_Done_FreeType(ft_lib);
        
    }
}
namespace BtkFt{
    Face::Face(const char *fname,FaceIndex index){
        std::lock_guard locker(Instance().mtx);

        FT_Error err = FT_New_Face(Instance().ft_lib,fname,index,&face);
        if(err){
            //...
        }
        name = face->family_name;
    }
    Face::Face(const void *buf,size_t buflen,FaceIndex index,bool dup){
        std::lock_guard locker(Instance().mtx);
        
        FT_Error err = FT_New_Memory_Face(
            Instance().ft_lib,
            static_cast<const FT_Byte*>(buf),
            buflen,
            index,
            &face
        );
        
        name = face->family_name;

        
        if(dup){
            font_mem = malloc(buflen);
            if(font_mem == nullptr){
                //fail to alloc
                //...
            }
            memcpy(font_mem,buf,buflen);
            should_free_mem = true;
        }
        else{
            font_mem = nullptr;
            should_free_mem = false;
        }
    }
    Face::~Face(){
        std::lock_guard locker(Instance().mtx);
        FT_Done_Face(face);
        if(should_free_mem and font_mem != nullptr){
            free(font_mem);
        }
    }
    bool Face::has_glyph(Char ch){
        return FT_Get_Char_Index(face,ch) != 0;
    }
    bool Face::render_glyph(CharIndex idx){
        return FT_Load_Glyph(face,idx,FT_LOAD_RENDER);
    }
    CharIndex Face::index_char(Char ch){
        return FT_Get_Char_Index(face,ch);
    }
    int Face::kerning_size(CharIndex prev,CharIndex cur){
        FT_Vector vec;
        FT_Get_Kerning(face, prev, cur, FT_KERNING_DEFAULT, &vec);
        return vec.x >> 6;
    }
    //Global metrics
    FontMetrics Face::metrics(){
        FontMetrics m;
        if(FT_IS_SCALABLE(face)){
            FT_Fixed scale = face->size->metrics.y_scale;
            if(scale == 0){
                m.ascender = face->ascender >> 6;
                m.descender = face->descender >> 6;
                m.height = (face->ascender - face->descender) >> 6;
            }
            else{
                m.ascender =  FT_MulFix(face->ascender,scale);
                m.descender = FT_MulFix(face->descender,scale);
                m.height =    FT_MulFix(face->ascender - face->descender,scale);
            }
        }
        else{
            m.ascender = face->ascender >> 6;
            m.descender = face->descender >> 6;
            m.height = (face->ascender - face->descender) >> 6;
        }
        return m;
    }
    GlyphMetric Face::glyph_metrics(CharIndex idx){
        FT_Load_Glyph(face,idx,FT_LOAD_DEFAULT);
        FT_GlyphSlot slot = face->glyph;
        GlyphMetric m;
        FT_Fixed adv;
        m.minx = slot->bitmap_left;
        m.maxx = m.minx + slot->bitmap.width;
        m.miny = -slot->bitmap_top;
        m.maxxy = m.miny + slot->bitmap.rows;
        FT_Get_Advance(face,idx,FT_LOAD_NO_SCALE,&adv);
        m.advance = adv;
        return m;
    }

    GlyphSlots::~GlyphSlots(){
        if(buffer != nullptr){
            std::free(buffer);
        }
    }
    GlyphSlots::GlyphSlots(const FT_GlyphSlot &s){
        
    }
}

namespace BtkFt{
    Size Font::text_size(std::string_view text){
        std::lock_guard locker(*face);

        std::string_view::iterator iter = text.begin();

        int w = 0;
        int h = 0;
        FT_UInt prev = 0;//Prev index
        FT_Vector kerning;
        FT_GlyphSlot slot = face->face->glyph;

        while(iter != text.end()){
            char32_t next = utf8::unchecked::next(iter);

            auto index = FT_Get_Char_Index(face->face,next);
            FT_Load_Glyph(face->face,index,FT_LOAD_DEFAULT);

            //Calc 
            FT_Get_Kerning(face->face,prev,index,FT_KERNING_DEFAULT,&kerning);
            w += kerning.x >> 6;

            auto m = face->glyph_metrics(index);

            h = std::max(h,m.maxxy - m.miny);
            w += m.maxx - m.minx;

            prev = index;
        }
        return {w,h};
    }
    PixBuf Font::render_glyph(Char ch,Color c){
        auto idx = face->index_char(ch);
        face->render_glyph(idx);

        FT_GlyphSlot slot = face->face->glyph;
        int w = slot->bitmap.width;
        int h = slot->bitmap.rows;
        PixBuf buf(w,h,SDL_PIXELFORMAT_RGBA32);

        BTK_ASSERT(slot->format == FT_PIXEL_MODE_GRAY);

        Uint32 *pixels = static_cast<Uint32*>(buf->pixels);
        //Copy into
        for(int i = 0;i < h;i++){
            for(int l = 0;l < w;l++){
                Uint8 alpha = slot->bitmap.buffer[i * w + l];

                alpha *= float(c.a) / 255;

                pixels[i * w + l] = SDL_MapRGBA(
                    buf->format,
                    c.r,
                    c.g,
                    c.b,
                    alpha
                );
            }
        }
        return buf;
    }
    int Font::kerning_size(char32_t prev,char32_t cur){
        auto i_prev = face->index_char(prev);
        auto i_cur = face->index_char(cur);

    }
    Font::Font(Face *face,float ptsize){
        this->face = face;
        this->ptsize = ptsize;
        this->refcount = 1;

        face->ref();
    }
}