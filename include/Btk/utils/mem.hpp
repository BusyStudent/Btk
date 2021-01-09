#if !defined(_BTKUTILS_MEM_HPP_)
#define _BTKUTILS_MEM_HPP_
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include "../defs.hpp"
namespace Btk{
    BTKAPI size_t U16Strlen(const char16_t *str);
    BTKAPI int U16Strcmp(const char16_t *s1,const char16_t *s2);
    BTKAPI int U16Strcasecmp(const char16_t *s1,const char16_t *s2);

    /**
     * @brief Convert u16string to u8string
     */
    BTKAPI void Utf16To8(std::string&,std::u16string_view);
    /**
     * @brief Convert u8string to u16string
     */
    BTKAPI void Utf8To16(std::u16string&,std::string_view);
    /**
     * @brief Check a string is vaid utf8
     * 
     */
    BTKAPI bool IsValidUtf8(std::string_view);
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
    T* Memdup(const T *ptr,size_t size){
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
    T* Memdup(const T *ptr){
        return Memdup<T,Alloc>(ptr,sizeof(T));
    }
    template<class T,auto Alloc = std::malloc>
    T* Memdup(const T &ref){
        return Memdup<T,Alloc>(&ref,sizeof(T));
    }
};

#endif // _BTKUTILS_MEM_HPP_
