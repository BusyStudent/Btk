#if !defined(_BTKUTILS_MEM_HPP_)
#define _BTKUTILS_MEM_HPP_
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "../defs.hpp"
namespace Btk{
    BTKAPI size_t U16Strlen(const char16_t *str);
    BTKAPI int U16Strcmp(const char16_t *s1,const char16_t *s2);
    BTKAPI int U16Strcasecmp(const char16_t *s1,const char16_t *s2);
    /**
     * @brief A helper template for dup memory
     * 
     * @tparam T The memory block type
     * @tparam Alloc The Allocator
     * @param ptr Source Pointer
     * @param size The memory block size
     * @return The new memory block
     */
    template<class T,auto Alloc = std::malloc>
    T* Memdup(T *ptr,size_t size){
        T* ret = static_cast<T*>(Alloc(size));
        if(ret == nullptr){
            return nullptr;
        }
        std::memcpy(ret,ptr,size);
        return ret;
    }
    /**
     * @brief A helper template for dup memory
     * 
     * @tparam T The memory block type
     * @tparam Alloc The Allocator
     * @param ptr Source Pointer
     * @return The new memory block
     */
    template<class T,auto Alloc = std::malloc>
    T* Memdup(T *ptr){
        return Memdup<T,Alloc>(ptr,sizeof(T));
    }
};

#endif // _BTKUTILS_MEM_HPP_
