#if !defined(_BTK_BUTTON_HPP_)
#define _BTK_BUTTON_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "themes.hpp"
#include "font.hpp"
#include "defs.hpp"
namespace Btk{
    class MouseEvent;
    /**
     * @brief Basic button
     * 
     */
    class BTKAPI AbstractButton:public Widget{
        public:
            //Process event
            bool handle(Event &) override;
            
            template<class ...T>
            void on_click(T &&...args){
                clicked.connect(std::forward<T>(args)...);
            }
            Signal<void()> &signal_clicked(){
                return clicked;
            }
            /**
             * @brief Get the button text
             * 
             * @return u8string_view 
             */
            u8string_view text() const{
                return btext;
            }
            PixBufRef     icon() const{
                return bicon;
            }
            bool is_checked() const noexcept{
                return checked;
            }

            // void set_parent(Widget *) override;
            void set_text(u8string_view view);
            void set_icon(PixBufRef     icon);
        protected:
            virtual void set_parent(Widget *) override;
            virtual void onenter();
            virtual void onleave();
            bool is_entered = false;//< Is mouse on the button?
            bool is_pressed = false;//< Is mouse pressed the button?
            float ptsize = 0;//< fontsize
            //For RadioButton and CheckButton
            bool checked = false;
            bool checkable = true;

            u8string btext;//< Button text
            PixBuf   bicon;//< Button icon
            Texture  bicon_tex;//< Button icon's texture

            Signal<void()> clicked;
    };
    /**
     * @brief A simple pushbutton
     * 
     */
    class BTKAPI Button:public AbstractButton{
        public:
            Button();
            Button(u8string_view text);
            Button(int x,int y,int w,int h);
            ~Button();

            void draw(Renderer &) override;
        protected:
            bool handle_mouse(MouseEvent &) override;
            void onleave() override;
            //PixBuf  textbuf;
            //Texture texture;
            //Font    textfont;
    };
    class BTKAPI RadioButton:public AbstractButton{
        public:
            using Widget::set_rect;

            RadioButton();
            //FIXME MSVC could found vtable here
            RadioButton(u8string_view text);
            ~RadioButton();

            void draw(Renderer &) override;
            void set_rect(const Rect &r) override;
            /**
             * @brief Config the circle's r
             * 
             * @param r 
             */
            void set_cirlce_r(float r){
                circle_r = r;
                redraw();
            }
        protected:
            bool handle_motion(MotionEvent &event) override;
            bool handle_mouse(MouseEvent &event) override;
        private:
            FPoint circle_center = {0,0};
            float  circle_r = 0;
            /**
             * @brief Text center
             * 
             */
            FPoint text_center;
    };
    class BTKAPI CheckButton:public AbstractButton{
        public:
            using Widget::set_rect;
            
            CheckButton();
            CheckButton(u8string_view text){
                btext = text;
            }
            ~CheckButton();
            void draw(Renderer &) override;
            //Event handle
            void set_rect(const Rect &r) override;
            bool handle_motion(MotionEvent &event) override;
            bool handle_mouse(MouseEvent &event) override;
        private:
            //The rect to check
            FRect  check_rect;
            FPoint text_center;
            bool   tristate = false;
            Uint8  status;
    };
}
#endif // _BTK_BUTTON_HPP_
