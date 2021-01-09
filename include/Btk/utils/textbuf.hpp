#if !defined(_BTKUTILS_TEXTBUFFER_HPP_)
#define _BTKUTILS_TEXTBUFFER_HPP_
#include <vector>
#include <string>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iosfwd>
#include <string_view>
#include <type_traits>

#include "../defs.hpp"
#include "./mem.hpp"

static_assert(sizeof(char16_t) == sizeof(uint16_t),"sizeof(char16_t) != sizeof(Uint16)");

namespace Btk{
    class BTKAPI TextSelection{
        
        friend class TextBuffer;
    };
    /**
     * @brief A container of utf16 encoded string for TextEditer
     */
    class BTKAPI TextBuffer{
        public:
            using value_type = std::u16string;
            using iterator = long;
        public:
            template<class ...Args>
            TextBuffer(Args &&...args):
                data(std::forward<Args>(args)...){};
            TextBuffer(const TextBuffer&) = default;
            ~TextBuffer();

            TextBuffer &append(std::u16string_view str){
                data += str;
                return *this;
            }
            TextBuffer &append(std::string_view str);

            std::u16string *operator ->() noexcept{
                return &data;
            }
        private:
            std::u16string data;//< The string container
        friend class TextSelection;
    };
};
#endif // _BTKUTILS_TEXTBUFFER_HPP_
