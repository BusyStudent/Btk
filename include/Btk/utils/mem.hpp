#if !defined(_BTKUTILS_MEM_HPP_)
#define _BTKUTILS_MEM_HPP_
#include <cstddef>
#include "../defs.hpp"
namespace Btk{
    BTKAPI size_t U16Strlen(const char16_t *str);
    BTKAPI int U16Strcmp(const char16_t *s1,const char16_t *s2);
    BTKAPI int U16Strcasecmp(const char16_t *s1,const char16_t *s2);
};

#endif // _BTKUTILS_MEM_HPP_
