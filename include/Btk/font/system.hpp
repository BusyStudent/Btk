#if !defined(_BTK_FONT_SYSTEM_HPP_)
#define _BTK_FONT_SYSTEM_HPP_
#include "../string.hpp"
#include "font.hpp"
#include <map>
namespace Btk::Ft{
    struct CacheSystem{
        /**
         * @brief name:face
         * 
         */
        std::map<u8string,Face> faces;

        bool  load_face(Face &,const char *filename,Uint32 idx);
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
    CacheSystem &GlobalCache();
}

#endif // _BTK_FONT_SYSTEM_HPP_
