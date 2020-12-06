#if !defined(_BTK_TEXTBOX_HPP_)
#define _BTK_TEXTBOX_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    class TextBox:public Widget{
        public:
            TextBox(Window&);
            TextBox(const TextBox &) = delete;
            ~TextBox();
        private:
            Font tfont;//Text Font
            
    };
};


#endif // _BTK_TEXTBOX_HPP_
