#if !defined(_BTK_LABEL_HPP_)
#define _BTK_LABEL_HPP_
#include <string>
#include <string_view>
#include "widget.hpp"
#include "pixels.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    /**
     * @brief Label widget
     * 
     */
    class BTKAPI Label:public Widget{
        public:
            Label();
            Label(std::string_view text);
            /**
             * @brief Construct a new Label object
             * 
             * @param x Label x
             * @param y Label y
             * @param w Label w
             * @param h Label h
             */
            Label(int x,int y,int w,int h);
            ~Label();
            std::string text() const{
                return text_;
            };
            //Set Label text
            void set_text(std::string_view text);
            void set_parent(Widget *) override;
            void draw(Renderer&);
        private:
            std::string text_;//text
            //Font font_;//font
            float ptsize = 0;
            
            //PixBuf text_buf;//text pixels buf
            //Texture texture;//text texture
            Color text_color = {0,0,0,255};//text color

            Align v_align = Align::Left;
            Align h_align = Align::Center;
    };
};

#endif // _BTK_LABEL_HPP_
