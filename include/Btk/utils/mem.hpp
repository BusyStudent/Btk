#if !defined(_BTKUTILS_MEM_HPP_)
#define _BTKUTILS_MEM_HPP_
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include "../string.hpp"
#include "../defs.hpp"
namespace Btk{
    /**
     * @brief Convert u16string to u8string
     * 
     * @return The string length we inserted
     */
    BTKAPI size_t Utf16To8(u8string&,u16string_view);
    /**
     * @brief Convert u8string to u16string
     * 
     * @return The string length we inserted
     */
    BTKAPI size_t Utf8To16(u16string&,u8string_view);
    /**
     * @brief Parse a hex string
     * 
     * @param txt The hex text
     * @return Sint64,INT64_MAX on error
     */
    BTKAPI Sint64 ParseHex(u8string_view txt);
    BTKAPI Sint64 ParseInt(u8string_view txt);
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
    /**
     * @brief Convert utf16 string to utf8 strubg
     * 
     * @param utf16 Utf16 encoded string
     * @return Utf8 encoded string
     */
    inline u8string Utf16To8(u16string_view utf16){
        u8string utf8;
        Utf16To8(utf8,utf16);
        return utf8;
    }
    /**
     * @brief Convert utf8 string to utf16 strubg
     * 
     * @param utf8 Utf8 encoded string
     * @return Utf16 encoded string
     */
    inline u16string Utf8To16(u8string_view utf8){
        u16string utf16;
        Utf8To16(utf16,utf8);
        return utf16;
    }
};

#endif // _BTKUTILS_MEM_HPP_
