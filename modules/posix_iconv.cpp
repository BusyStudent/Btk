#include <Btk/module.hpp>
#include <Btk/string.hpp>

#include <iconv.h>

BTK_MODULE_INIT(module_info){
    module_info.name = "libiconv";
    
    using Btk::IconvFunctions;
    using Btk::HookIconv;

    IconvFunctions fns;
    //Replace iconv functions
    fns.iconv = reinterpret_cast<decltype(IconvFunctions::iconv)>(::iconv);
    fns.iconv_open = reinterpret_cast<decltype(IconvFunctions::iconv_open)>(::iconv_open);
    fns.iconv_close = reinterpret_cast<decltype(IconvFunctions::iconv_close)>(::iconv_close);

    HookIconv(fns);
}