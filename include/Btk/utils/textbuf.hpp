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
     * @FIXME I donnot why iconv will skip 2 byte a the buffer begin
     *        Is this bug?
     */
    class BTKAPI TextBuffer{
        public:
            using iterator = char16_t*;
            using const_iterator = const char16_t*;
            using value_type = char16_t;
            using reference = char16_t&;
            using const_reference = const char16_t&;
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
            void clear();
            /**
             * @brief add content in the buf's end
             * 
             * @param u16 The data
             */
            void append(std::u16string_view u16);
            void append(std::string_view u16);
            void append(const char16_t *u16str);
            void append(const char16_t *u16str,size_t len);
            void append(const char *u8);
            void append(const char *u8,size_t len);
            void append(const TextBuffer &buffer);
            void append(char16_t ch);
            void append(char ch);
            /**
             * @brief Assign the buffer
             * 
             * @param buffer Another buffer
             */
            void assign(const TextBuffer &buffer);
            void assign(TextBuffer &&buffer);
            /**
             * @brief Extend the buffer
             * @note If new_len < max_len;It is no-op
             * @param new_len The new length
             */
            void extend(size_t new_len);

            template<class ...Args>
            void push_back(Args &&...args){
                append(std::forward<Args...>(args)...);
            }
            void pop_back();

            void shrink_to_fit();
            /**
             * @brief Set Buffer size
             * @note If the new_len < len The String will be tuncated
             * 
             * @param new_len The new len
             */
            void resize(size_t new_len);
            size_t length() const noexcept{
                return len;
            }
            /**
             * @brief Convert to std::string
             * 
             * @return utf8 encoded string
             */
            std::string to_string() const;
            operator std::u16string_view() const noexcept{
                return std::u16string_view(mem,len);
            };
        public:
            //iterator
            iterator begin(){
                return mem + 1;
            }
            iterator end(){
                return mem + len;
            }
            iterator begin() const{
                return mem + 1;
            }
            iterator end() const{
                return mem + len;
            }

            reference operator [](size_t len){
                return mem[len + 1];
            }
            const_reference operator [](size_t len) const{
                return mem[len + 1];
            }
        public:
            template<class ...Args>
            TextBuffer &operator +=(Args &&...args){
                append(std::forward<Args>(args)...);
                return *this;
            }
            TextBuffer &operator +=(const TextBuffer &);
            TextBuffer &operator +=(TextBuffer &&);
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
            operator std::string() const{
                return to_string();
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
    inline void TextBuffer::append(char16_t ch){
        append(&ch,1);
    }
    inline void TextBuffer::append(char ch){
        append(&ch,1);
    }

    inline void TextBuffer::pop_back(){
        if(len != 0){
            --len;
            if(mem != nullptr){
                mem[len] = u'\0';
            }
        }
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
