#if !defined(_BTK_FT_INTERNAL_HPP_)
#define _BTK_FT_INTERNAL_HPP_
#include <cassert>
#include <cstring>
#include <mutex>
#include <map>

//SDL2
#include <SDL2/SDL_atomic.h>

//Freetype impl
#include <ft2build.h>
#include FT_FREETYPE_H

#include <Btk/defs.hpp>

namespace BTKHIDDEN BtkFt{
    using FaceIndex = FT_ULong;
    using CharIndex = FT_ULong;
    using Char = char32_t;
    struct Font;
    struct FontMetrics{
        float ascender;
        float descender;
        float height;
    };
    struct Face{
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
         * @brief Is the glyph is provieded
         * 
         * @param ch 
         * @return true 
         * @return false 
         */
        bool has_glyph(Char ch);

        void render_glyph();
    };
    /**
     * @brief Slot for 
     * 
     */
    struct GlyphSlots{
        int w;
        int h;
        int pitch;
        Uint8 *buffer;
    };
    /**
     * @brief The font user used
     * 
     */
    struct Font{
        Face *face;
        int refcount;
    };
    struct Library{
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
    extern void Init();
    extern void Quit();
    extern Library *library;
    inline Library &Instance(){
        return *library;
    }
}


#endif // _BTK_FT_INTERNAL_HPP_
