#if !defined(_BTK_FONT_SYSTEM_HPP_)
#define _BTK_FONT_SYSTEM_HPP_
#include "../string.hpp"
#include "font.hpp"
#include <map>
namespace Btk::Ft{
    struct BTKHIDDEN CacheSystem{
        /**
         * @brief name:face
         * 
         */
        std::map<u8string,Face> faces;
        std::map<u8string,FontBuffer> memfonts;

        bool  load_face(Face &,const char *filename,Uint32 idx);
        /**
         * @brief Load a font
         * 
         * @param name The font name
         * @param idx The index
         * @return Font 
         */
        Font *load_font(const u8string &name,Uint32 idx);
        /**
         * @brief Query the font name
         * 
         * @param name The font name
         * @return Font* nullptr on error
         */
        Font *query(const u8string &name);
    };
    /**
     * @brief Get cache system
     * 
     * @return CacheSystem& 
     */
    BTKHIDDEN CacheSystem &GlobalCache();
}

#endif // _BTK_FONT_SYSTEM_HPP_
