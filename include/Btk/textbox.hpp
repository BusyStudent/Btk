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
    class BTKAPI AbstractEditor:public Widget{
        public:
            AbstractEditor();
            ~AbstractEditor();

            //Default event handling
            bool handle_drag(DragEvent   &) override;
            bool handle_mouse(MouseEvent &) override;
            bool handle_keyboard(KeyEvent &) override;

            struct _Internal;
        private:
            _Internal *context;
        protected:
            void editor_init(bool single_lines);
            void editor_click(float x,float y);

            u8string editor_cur_insert();
            u8string editor_cur_delete();
            //Callback
            virtual bool editor_on_insert(size_t position){return true;};
            virtual bool editor_on_delete(size_t position){return true;};

        friend struct _Internal;
    };
    class BTKAPI TextEdit:public AbstractEditor{
        private:
            u8string cur_text;
    };
    /**
     * @brief Just edit single line
     * 
     */
    class BTKAPI LineEdit:public Widget{
        public:
            LineEdit();
            LineEdit(u8string_view text);
            ~LineEdit();
            
            void draw(Renderer &) override;
            
            bool handle(Event &) override;
            bool handle_drag(DragEvent &) override;
            bool handle_mouse(MouseEvent &) override;
            bool handle_keyboard(KeyEvent &) override;
            bool handle_textinput(TextInputEvent&) override;
        private:
            Color    text_color;
            Color    background_color;
            Color    boarder_color;
            u8string placeholder;//< Show if the string is empty
            u8string cur_text;//< Current text
            Align    align;//< Text align
            bool     clear_btn = false;//Has clear button?
            //select
            int sel_begin;
            int sel_end;

    };
    class BTKAPI TextBroser:public Widget{
        
    };
};


#endif // _BTK_TEXTBOX_HPP_
