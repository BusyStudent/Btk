#if !defined(_BTK_TEXTBOX_HPP_)
#define _BTK_TEXTBOX_HPP_
#include <string>
#include <string_view>

#include "widget.hpp"
#include "pixels.hpp"
#include "themes.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    class KeyEvent;
    class TextBox:public Widget{
        public:
            TextBox(Window&);
            TextBox(const TextBox &) = delete;
            ~TextBox();

            bool handle(Event &);
            void draw(Renderer &);
            /**
             * @brief Get TextBox's text
             * 
             * @return UTF16 encoded string
             */
            std::u16string_view text() const{
                return tb_text;
            }
            std::string u8text() const{
                std::string s;
                u8text(s);
                return s;
            }
            /**
             * @brief Get TextBox's utf8 text
             * 
             * @param str The UTF8 encoded container
             */
            void u8text(std::string &str) const;
            /**
             * @brief Set the text 
             * 
             * @param txt The UTF16 encoded string 
             */
            void set_text(std::u16string_view txt);
            void set_text(std::string_view txt);
        private:
            //Process keyboard event
            void do_keyboard(KeyEvent &event);
            Font  tb_font;//Text Font
            Theme theme;
            
            std::u16string tb_text;//Text
            PixBuf  tb_buf;//Rendered text
            Texture texture;

            bool has_focus = false;
            std::u16string::iterator cur_txt;//Current text

            int ft_h;//Rendered Text's h
            int tb_boarder = 4;//The text boarder
    };
};


#endif // _BTK_TEXTBOX_HPP_
