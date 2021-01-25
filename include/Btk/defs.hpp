#if !defined(_BTK_DEFS_HPP_)
#define _BTK_DEFS_HPP_

#ifdef __cplusplus
    #include <cstdint>
#else
    #include <stdint.h>
#endif

#ifdef _WIN32
    #ifdef _MSC_VER
    #define BTKEXPORT __declspec(dllexport)
    #define BTKIMPORT __declspec(dllimport)
    
    #undef not
    #undef and
    #undef or

    #define not !
    #define and &&
    #define or ||
    #else
    #define BTKEXPORT __attribute__((dllexport))
    #define BTKIMPORT __attribute__((dllimport))
    #endif
#else
    //ignore this
    #define BTKEXPORT     
    #define BTKIMPORT 
#endif

#ifdef _BTK_SOURCE
    //source file
    #define BTKAPI BTKEXPORT
#else
    #define BTKAPI BTKIMPORT
#endif

#ifdef __cplusplus
namespace Btk{
    //Some int defs in SDL2
    //Uint
    using Uint8 = uint8_t;
    using Uint16 = uint16_t;
    using Uint32 = uint32_t;
    using Uint64 = uint64_t;
    //Sint
    using Int8 = int8_t;
    using Int16 = int16_t;
    using Int32 = int32_t;
    using Int64 = int64_t;

    using Sint8 = int8_t;
    using Sint16 = int16_t;
    using Sint32 = int32_t;
    using Sint64 = int64_t;
    //end
};
#endif

#endif // _BTK_DEFS_HPP_
