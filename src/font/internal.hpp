#if !defined(_BTK_FT_INTERNAL_HPP_)
#define _BTK_FT_INTERNAL_HPP_
#include <cassert>
#include <cstring>
#include <string>
#include <mutex>
#include <map>

//SDL2
#include <SDL2/SDL_atomic.h>

//Freetype impl
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <Btk/impl/atomic.hpp>
#include <Btk/pixels.hpp>
#include <Btk/rect.hpp>
#include <Btk/defs.hpp>

namespace BtkFt{
    using FaceIndex = FT_ULong;
    using CharIndex = FT_ULong;
    using Char = char32_t;
    using Btk::Size;
    using Btk::Color;
    using Btk::PixBuf;
    using Btk::Atomic;
    struct Font;
    struct FontMetrics{
        float ascender;
        float descender;
        float height;
    };
    struct GlyphMetric{
        int minx;
        int maxx;
        int miny;
        int maxxy;
        int advance;
    };
    struct Glyph;
    struct Bitmap;
    /**
     * @brief The font's data buffer
     * 
     */
    struct FaceData{
        void *buffer;
        size_t len;
        Atomic refcount;
    };
    struct BTKHIDDEN Face{
        Face(const char *fname,FaceIndex index = 0);
        Face(const void *buf,size_t buflen,FaceIndex index = 0,bool dup = true);
        ~Face();
        FT_Face face;
        std::string name;

        int refcount = 1;
        int spinlock = 0;

        void *font_mem = nullptr;
        bool should_free_mem = false;

        void unref();
        void ref(){
            ++refcount;
        }
        void lock(){
            SDL_AtomicLock(&spinlock);
        }
        void unlock(){
            SDL_AtomicUnlock(&spinlock);
        }
        /**
         * @brief Get global metrics
         * 
         * @return FontMetrics 
         */
        FontMetrics metrics();
        /**
         * @brief Get GlyphMetric
         * 
         * @param index 
         * @return GlyphMetric 
         */
        GlyphMetric glyph_metrics(CharIndex index);
        /**
         * @brief Get the char's index
         * 
         * @param ch 
         * @return Index(0 on failure)
         */
        CharIndex index_char(Char ch);
        /**
         * @brief Is the glyph is provieded
         * 
         * @param ch 
         * @return true 
         * @return false 
         */
        bool has_glyph(Char ch);
        bool render_glyph(CharIndex index);
        /**
         * @brief Get the glyph object
         * 
         * @param index The char index
         * @return Glyph 
         */
        Glyph get_glyph(CharIndex index);
        int kerning_size(CharIndex prev,CharIndex cur);
    };
    /**
     * @brief Glyph
     * 
     */
    struct BTKHIDDEN Glyph{
        Glyph(FT_Glyph g):glyph(g){}
        Glyph(const Glyph &g){
            FT_Glyph_Copy(g.glyph,&glyph);
        }
        Glyph(Glyph &&g){
            glyph = g.glyph;
            g.glyph = nullptr;
        }
        ~Glyph(){
            FT_Done_Glyph(glyph);
        }
        FT_Glyph operator ->() const noexcept{
            return glyph;
        }
        FT_Glyph glyph;
    };
    /**
     * @brief GlyphBitmap
     * 
     */
    struct BTKHIDDEN Bitmap{
        Bitmap(FT_BitmapGlyph g):glyph(g){}
        Bitmap(const Bitmap &map){
            FT_Glyph_Copy(
                reinterpret_cast<FT_Glyph>(map.glyph),
                reinterpret_cast<FT_Glyph*>(&glyph)
            );
        }
        Bitmap(Bitmap && b):glyph(b.glyph){
            b.glyph = nullptr;
        }
        ~Bitmap(){
            FT_Done_Glyph(reinterpret_cast<FT_Glyph>(glyph));
        }
        FT_BitmapGlyph operator ->() const noexcept{
            return glyph;
        }
        FT_BitmapGlyph glyph;
    };
    /**
     * @brief Slot for 
     * 
     */
    struct BTKHIDDEN GlyphSlots{
        ~GlyphSlots();
        /**
         * @brief Copy from FT_GlyphSlot
         * 
         */
        GlyphSlots(const FT_GlyphSlot &);
        GlyphSlots(const GlyphSlots &) = delete;
        GlyphSlots(GlyphSlots &&s){
            w = s.w;
            h = s.h;
            pitch = s.pitch;
            buffer = s.buffer;

            s.buffer = nullptr;
        }
        GlyphSlots();
        int w;
        int h;
        int pitch;
        Uint8 *buffer;
    };
    /**
     * @brief The font user used
     * 
     */
    struct BTKHIDDEN Font{
        /**
         * @brief Construct a new Font object
         * 
         * @param name 
         * @param ptsize 
         */
        Font(std::string_view name,float ptsize);
        Font(const Font &f){
            face = f.face;
            refcount = 1;
            face->ref();
            ptsize = f.ptsize;
        }
        Font(Face *face,float ptsize);
        int refcount = 0;
        Face *face = nullptr;
        float ptsize = 0;
        int style = 0;

        Size text_size(std::string_view text);
        Size text_size(std::u16string_view text);

        void ref(){
            ++refcount;
        }
        void unref(){
            --refcount;
            if(refcount <= 0){
                face->unref();
                delete this;
            }
        }

        PixBuf render_glyph(Char ch,Color color);
        PixBuf render_text(std::string_view text,Color color);
        /**
         * @brief Get kerning size
         * 
         * @param prev_ch 
         * @param ch 
         * @return int 
         */
        int kerning_size(char32_t prev_ch,char32_t ch);
        bool has_glyph(char32_t ch){
            return face->has_glyph(ch);
        }

        Glyph get_glyph(Char ch){
            auto i = face->index_char(ch);
            
        }

        std::string family_name() const{
            return face->face->family_name;
        }
        std::string style_name() const{
            return face->face->style_name;
        }
        float height() const{
            return face->metrics().height;
        }
    };
    struct BTKHIDDEN Library{
        Library();
        ~Library();
        Library(const Library &) = delete;
        /**
         * @brief Freetype library
         * 
         */
        FT_Library ft_lib;
        //Mutex for 
        std::recursive_mutex mtx;

        std::map<std::string,Face*> faces_map;
        /**
         * @brief Find a font
         * 
         * @param fontname 
         * @return Face* 
         */
        Face *find_font(std::string_view fontname);
        /**
         * @brief Add a font
         * 
         * @param mem 
         * @param bufn 
         * @param index 
         * @param duplicate 
         * @return Face* 
         */
        Face *add_mem_font(const void *mem,size_t bufn,FaceIndex index,bool duplicate);
        /**
         * @brief Add a font
         * 
         * @param fname 
         * @param index 
         * @return Face* 
         */
        Face *add_font(const char *fname,FaceIndex index);
    };
    struct FontStash{

    };
    extern BTKHIDDEN void Init();
    extern BTKHIDDEN void Quit();
    extern BTKHIDDEN Library *library;
    inline Library &Instance(){
        return *library;
    }
}


#endif // _BTK_FT_INTERNAL_HPP_
