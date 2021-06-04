#if !defined(_BTK_TEXTBOX_HPP_)
#define _BTK_TEXTBOX_HPP_
#include <string>
#include <string_view>
#include "utils/timer.hpp"
#include "widget.hpp"
#include "pixels.hpp"
#include "themes.hpp"
#include "rect.hpp"
#include "font.hpp"
namespace Btk{
    class KeyEvent;
    class BTKAPI TextBox:public Widget{
        public:
            TextBox();
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
            void set_text(u8string_view txt);
            void set_parent(Widget *w);
        private:
            void timeout();
            //Process keyboard event
            bool handle_drag(DragEvent    &) override;
            bool handle_mouse(MouseEvent  &) override;
            bool handle_keyboard(KeyEvent &) override;
            bool handle_textinput(TextInputEvent&) override;
            //Add string in where the cur_text point
            void add_string(u8string_view);
            //Font  tb_font;//Text Font
            float ptsize;//< Font ptsize
            Theme theme;
            
            std::u16string tb_text;//Text
            //PixBuf  tb_buf;//Rendered text
            //Texture texture;

            bool has_focus = false;//Flag of has focus
            bool is_dragging = false;//< Flag of drag
            bool show_line = true;//<Flag of show the edit line

            std::u16string::iterator cur_txt;//Current text
            //It will be point from tb_text.begin() - 1
            //to tb_text.end() - 1
            std::u16string::iterator sel_begin;//select text begin
            std::u16string::iterator sel_end;//select text end

            Timer timer;//For drawing the line
            float ft_h;//Rendered Text's h
            float tb_boarder = 4;//The text boarder
        friend struct TextBoxInserter;
    };
};


#endif // _BTK_TEXTBOX_HPP_
