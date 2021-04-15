#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ERRORS_H

#include <cmath>
#include <algorithm>

#include "internal.hpp"

namespace BtkFt{
    Library::Library(){
        FT_Error err = FT_Init_FreeType(&ft_lib);
        assert(err == 0);
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
    //Global metrics
    FontMetrics Face::metrics(){
        FontMetrics m;
        if(FT_IS_SCALABLE(face)){
            FT_Fixed scale = face->size->metrics.y_scale;

            m.ascender =  FT_MulFix(face->ascender,scale);
            m.descender = FT_MulFix(face->descender,scale);
            m.height =    FT_MulFix(face->ascender - face->descender,scale);
        }
        else{
            m.ascender = face->size->metrics.ascender;
            m.descender = face->size->metrics.descender;
            m.height = face->size->metrics.height;
        }
        return m;
    }
}