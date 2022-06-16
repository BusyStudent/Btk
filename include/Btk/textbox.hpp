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
    #if 0
    class BTKAPI AbstractEditor:public Widget{
        public:
            AbstractEditor();
            ~AbstractEditor();

            //Default event handling
            bool handle_drag(DragEvent   &) override;
            bool handle_mouse(MouseEvent &) override;
            bool handle_keyboard(KeyEvent &) override;

            struct _Internal;
        protected:
            _Internal *context;
        protected:
            void editor_init(bool single_lines);
            void editor_click(float x,float y);
            void editor_draw(float x,float y,float w,float h);

            u8string editor_cur_insert();
            u8string editor_cur_delete();
            //Callback
            virtual bool editor_on_insert(size_t position){return true;};
            virtual bool editor_on_delete(size_t position){return true;};

        friend struct _Internal;
    };
    class BTKAPI TextEdit:public AbstractEditor{
        public:
            void draw(Renderer &) override;
        private:
            u8string cur_text;
    };
    #endif
    /**
     * @brief Just edit single line
     * 
     */
    class BTKAPI LineEdit:public Widget{
        public:
            LineEdit();
            LineEdit(u8string_view text);
            ~LineEdit();
            
            using Widget::set_rect;
            
            void draw(Renderer &,Uint32) override;
            
            bool handle(Event &) override;
            bool handle_drag(DragEvent &) override;
            bool handle_mouse(MouseEvent &) override;
            bool handle_keyboard(KeyEvent &) override;
            bool handle_textinput(TextInputEvent&) override;

            void set_rect(const Rect &r) override;
            void set_text(u8string_view txt);
            void set_placeholder(u8string_view txt);

            bool has_selection() const noexcept{
                return has_sel and (sel_beg != sel_end);
            }

            u8string_view text() const{
                return cur_text;
            }
            u8string_view selection() const;

            Signal<void()> &signal_enter_pressed(){
                return _signal_enter_pressed;
            }
            Signal<void()> &signal_value_changed(){
                return _signal_value_changed;
            }
        private:
            //Utils
            auto get_pos_from(const Point &p) -> size_t;
            void do_paste(u8string_view);
            /**
             * @brief Delete a range of text
             * 
             * @param start 
             * @param end 
             */
            void do_delete(size_t start,size_t end);
            /**
             * @brief Delete text at
             * 
             * @param pos 
             */
            void do_erase(size_t pos){
                do_delete(pos,pos + 1);
            }
            /**
             * @brief Get Range of Selection
             * 
             * @return std::pair<size_t,size_t> 
             */
            auto sel_range() const -> std::pair<size_t,size_t>{
                size_t start = min(sel_beg,sel_end);
                size_t end   = max(sel_beg,sel_end);
                return {start,end};
            }
            void clear_sel();
            //Area
            //(X,Y)----------X>
            //|  |  LimitArea|
            //|  |           |
            //Y----------------
            FMargin limit_margin = {
                2.0f,
                2.0f,
                2.0f,
                2.0f
            };
            FPoint text_pos = {
                0,
                0
            };
            //Text Iterators
            //
            //  H e l l o W o r l d
            //0 1 2 3 4 5 6 7 8 9 10
            //Section
            size_t sel_beg = 0;
            size_t sel_end = 0;
            bool has_sel = false;
            bool has_focus = false;
            //Cursor
            bool show_cur = false;
            size_t cur_pos = 0;

            Color    text_color;
            Color    background_color;
            Color    boarder_color;
            u8string placeholder;//< Show if the string is empty
            u8string cur_text;//< Current text
            Align    align = Align::Left | Align::Middle;//< Text align
            bool     clear_btn = false;//Has clear button?
            //select
            //Signal
            Signal<void(u8string_view t)> _signal_value_deleted;
            Signal<void(               )> _signal_value_changed;
            Signal<void(               )> _signal_enter_pressed;
    };
    class BTKAPI TextBroser:public Widget{
        
    };
};


#endif // _BTK_TEXTBOX_HPP_
