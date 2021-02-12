#if !defined(_BTK_FT_GDI_HPP_)
#define _BTK_FT_GDI_HPP_
#include <basetsd.h>
#include <wingdi.h>
#include <windows.h>
#include "../utils/string.hpp"
namespace BtkFt{
    using Btk::u16string;
    /**
     * @brief Windows DGIFont impl
     * 
     */
    struct DGIFont{
        DGIFont() = default;
        ~DGIFont();
        /**
         * @brief Open font by its name
         * 
         * @param facename 
         * @param ptsize 
         * @return true 
         * @return false 
         */
        bool open(const u16string &facename,int ptsize);
        HFONT font = nullptr;
    };
}


#endif // _BTK_FT_GDI_HPP_
