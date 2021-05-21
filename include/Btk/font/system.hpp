#if !defined(_BTK_FONT_SYSTEM_HPP_)
#define _BTK_FONT_SYSTEM_HPP_
#include "../string.hpp"
#include "font.hpp"
#include <map>
namespace Btk::Ft{
    struct CacheSystem{
        std::map<u8string,Face> faces;
    };
    extern CacheSystem *GlobalCache;
}

#endif // _BTK_FONT_SYSTEM_HPP_
