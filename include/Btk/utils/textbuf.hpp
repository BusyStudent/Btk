#if !defined(_BTKUTILS_TEXTBUFFER_HPP_)
#define _BTKUTILS_TEXTBUFFER_HPP_
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include "../defs.hpp"
#include "./mem.hpp"

static_assert(sizeof(char16_t) == sizeof(uint16_t),"sizeof(char16_t) != sizeof(Uint16)");

namespace Btk{
    /**
     * @brief A container of utf16 encoded string
     * 
     */
    class BTKAPI TextBuffer{
        public:
            using iterator = char16_t*;
            using const_iterator = const char16_t*;
            /**
             * @brief Construct a new empty Text Buffer object
             * 
             */
            TextBuffer():mem(nullptr),len(0),max_len(0){};
            /**
             * @brief Construct a new Text Buffer object
             * 
             * @tparam T The first arg type
             * @tparam Args The next args type
             * @param arg1 The first arg
             * @param args The next args
             */
            template<class T,class ...Args>
            TextBuffer(T &&arg1,Args &&...args):TextBuffer(){
                append(std::forward<T>(arg1),std::forward<Args>(args)...);
            }
            TextBuffer(const TextBuffer &);
            TextBuffer(TextBuffer &&);
            ~TextBuffer();

            void append(std::u16string_view u16);
            void append(std::string_view u16);
            void append(const char16_t *u16str);
            void append(const char16_t *u16str,size_t len);
            void append(const char *u8);
            void append(const char *u8,size_t len);

            void push_back(char16_t ch);
            /**
             * @brief Convert to std::string
             * 
             * @return utf8 encoded string
             */
            std::string to_string() const;
            operator std::u16string_view() const noexcept{
                return std::u16string_view(mem,len);
            };

            /**
             * @brief To std::wstring_view
             * @note It only useable when sizeof(wchar_t) == sizeof(Uint16)
             * @return std::wstring_view
             */
            template<class T = 
                std::enable_if<
                    sizeof(wchar_t) == sizeof(Uint16),
                    std::wstring_view
                >
            >
            operator T() const noexcept{
                return std::wstring_view(
                    reinterpret_cast<const wchar_t*>(mem),
                    len
                );
            };

            template<class ...Args>
            TextBuffer &operator +=(Args &&...args){
                append(std::forward<Args>(args)...);
                return *this;
            }
            TextBuffer &operator +=(const TextBuffer &);
            TextBuffer &operator +=(TextBuffer &&);
            iterator begin(){
                return mem;
            }
            iterator end(){
                return mem + len;
            }
            iterator begin() const{
                return mem;
            }
            iterator end() const{
                return mem + len;
            }
            const char16_t *c_str() const noexcept{
                return mem;
            }
        private:
            char16_t *mem;//< The buffer's memory
            size_t  len;//< Current length
            size_t  max_len;//< The buffer capicity
    };

    //Append functions
    inline void TextBuffer::append(const char *utf8){
        append(utf8,strlen(utf8));
    }
    inline void TextBuffer::append(const char16_t *utf16){
        append(utf16,U16Strlen(utf16));
    }
    inline void TextBuffer::append(std::string_view utf8){
        append(utf8.data(),utf8.length());
    }
    inline void TextBuffer::append(std::u16string_view utf16){
        append(utf16.data(),utf16.length());
    }


    inline TextBuffer::TextBuffer(TextBuffer &&buf){
        mem = buf.mem;
        len = buf.len;
        max_len = buf.max_len;

        buf.mem = nullptr;
        buf.len = 0;
        buf.max_len = 0;
    }
};
#endif // _BTKUTILS_TEXTBUFFER_HPP_
