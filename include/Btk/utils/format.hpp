#if !defined(_BTK_UTILS_FORMAT_HPP_)
#define _BTK_UTILS_FORMAT_HPP_
#include "../defs.hpp"
#include "../string.hpp"
#include <charconv>
namespace Btk::FmtImpl{
    template<class T>
    struct Converter;

    template<>
    struct Converter<u8string_view>{
        static void do_cast(u8string &ctxt,u8string_view self){
            ctxt.append(self);
        }
    };
    template<>
    struct Converter<u8string>{
        static void do_cast(u8string &ctxt,const u8string &self){
            ctxt.append(self);
        }
    };
}

#endif // _BTK_UTILS_FORMAT_HPP_
