#if !defined(_BTK_LABLE_HPP_)
#define _BTK_LABLE_HPP_
#include <string>
#include <string_view>
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    /**
     * @brief Lable widget
     * 
     */
    class Lable:public Widget{
        public:
            Lable(Window&);
            Lable(Window&,std::string_view text);
            ~Lable();
            std::string text() const{
                return text_;
            };
            //Set Lable text
            void set_text(std::string_view text);
            void draw(Renderer&);
            bool handle(Event&);
        private:
            std::string text_;
            Font font_;
            PixBuf text_buf;
            Texture texture;
            Color text_color;

            Align v_align = Align::Left;
            Align h_align = Align::Center;
    };
};

#endif // _BTK_LABLE_HPP_
