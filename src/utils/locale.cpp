#include "../build.hpp"


#ifdef __linux
#include <langinfo.h>
namespace{
    std::string_view get_encoding(){
        return nl_langinfo(CODESET);
    }
}




#endif