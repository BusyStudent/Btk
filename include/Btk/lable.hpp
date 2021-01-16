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
    class BTKAPI Lable:public Widget{
        public:
            Lable(Container&);
            Lable(Container&,std::string_view text);
            /**
             * @brief Construct a new Lable object
             * 
             * @param x Lable x
             * @param y Lable y
             * @param w Lable w
             * @param h Lable h
             */
            Lable(Container&,int x,int y,int w,int h);
            ~Lable();
            std::string text() const{
                return text_;
            };
            //Set Lable text
            void set_text(std::string_view text);
            void draw(Renderer&);
        private:
            std::string text_;//text
            Font font_;//font
            
            PixBuf text_buf;//text pixels buf
            Texture texture;//text texture
            Color text_color;//text color

            Align v_align = Align::Left;
            Align h_align = Align::Center;
    };
};

#endif // _BTK_LABLE_HPP_
