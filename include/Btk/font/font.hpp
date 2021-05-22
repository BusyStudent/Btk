#if !defined(_BTK_FONT_FT_HPP_)
#define _BTK_FONT_FT_HPP_
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H

#include "../defs.hpp"
#include "../object.hpp"
#include "../exception.hpp"

#include <vector>
namespace Btk::Ft{
    using Char = FT_ULong;
    using CharIndex = FT_UInt;

    extern FT_Library Ft2Library;
    struct BTKHIDDEN FontBuffer{
        FontBuffer() = default;
        FontBuffer(const FontBuffer &) = default;
        ~FontBuffer() = default;

        RefPtr<std::vector<Uint8>> _data;

        Uint8 *data() const noexcept{
            return _data->data();
        }
        size_t size() const noexcept{
            return _data->size();
        }
    };
    /**
     * @brief Freetype exception
     * 
     */
    class Ft2Exception:public RuntimeError{
        
    };
    /**
     * @brief Freetype face
     * 
     */
    struct BTKHIDDEN Ft2Face{
        /**
         * @brief Construct a new Ft 2 Face object
         * 
         * @param fontdata The refptr to fontdata
         * @param idx The Face index
         */
        Ft2Face(FontBuffer fontdata,Uint32 idx);
        Ft2Face(const char *fname,Uint32 idx);
        Ft2Face(const Ft2Face &) = delete;
        ~Ft2Face();

        FontBuffer buffer;
        FT_Face face;
        SpinLock spinlock;
    };
    /**
     * @brief Raw face
     * 
     */
    struct BTKHIDDEN Face{
        RefPtr<Ft2Face> face;

        Face() = default;
        Face(const Face &) = default;
        ~Face() = default;
        
        operator FT_Face() const{
            return face->face;
        }
        FT_Face operator ->() const{
            return face->face;
        }
        void lock(){
            face->spinlock.lock();
        }
        void unlock(){
            face->spinlock.unlock();
        }
    };
    constexpr CharIndex InvaildCharIndex = 0;
    struct BTKHIDDEN Font{
        explicit Font(Face f):face(f){};
        explicit Font(const void *buf,size_t bufsize,Uint32 idx);
        explicit Font(const char *filename,Uint32 idx);

        Font(const Font &);
        ~Font();

        Face face;
        struct{
            int w = 0;
            int h = 0;
            int pitch = 0;
            Uint8 *buffer = nullptr;
        }bitmap;//< 256 grey bitmap
        float ptsize = 0;
        int refcount = 1;

        void ref(){
            ++refcount;
        }
        void unref(){
            --refcount;
            if(refcount <= 0){
                delete this;
            }
        }
        void set_ptsize(float s){
            ptsize = s;
        }
        void bitmap_free();
        /**
         * @brief realloc the bitmap
         * 
         * @param new_w 
         * @param new_h 
         */
        void bitmap_realloc(int new_w,int new_h);
        bool bitmap_render(CharIndex idx);
        int kerning_size(CharIndex prev,CharIndex cur);
        int advance(CharIndex idx);
        /**
         * @brief Get a char index
         * 
         * @param ch 
         * @return CharIndex 0 on failure
         */
        CharIndex index_char(Char ch);
        //operation for fontstash in nanovg
        void  fs_get_vmetrics(int *ascent,int *descent,int *lineGap);
        float fs_get_pixel_height_scale(float size);
        bool  fs_build_glyph(int glyph, float size, float scale,
							 int *advance, int *lsb, int *x0, int *y0, int *x1, int *y1);
        void  fs_render_glyph(unsigned char *output, int outWidth, int outHeight, int outStride,
							 float scaleX, float scaleY, int glyph);
    };
    Font *OpenFont(const void *buf,size_t n,Uint32 idx);
}

#endif // _BTK_FONT_FT_HPP_
