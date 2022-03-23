#if !defined(_BTK_BUTTON_HPP_)
#define _BTK_BUTTON_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "themes.hpp"
#include "font.hpp"
#include "defs.hpp"
namespace Btk{
    /**
     * @brief Basic button
     * 
     */
    class BTKAPI AbstractButton:public Widget{
        public:
            /**
             * @brief Construct a new Abstract Button object
             * @note It will init theme here
             */
            AbstractButton();

            //Process event
            bool handle(Event &) override;
            
            template<class ...T>
            [[deprecated("Use signal_clicked")]]
            void on_click(T &&...args){
                clicked.connect(std::forward<T>(args)...);
            }
            auto signal_clicked() -> Signal<void()> &{
                return clicked;
            }
            auto siganl_hovered() -> Signal<void()> &{
                return hovered;
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

            //< control draw style
            void set_draw_border(bool b){
                draw_border = b;
                redraw();
            }
            void set_draw_border_on_hover(bool b){
                draw_border_on_hover = b;
                redraw();
            }

        protected:
            virtual void set_parent(Widget *) override;
            virtual void onenter();
            virtual void onleave();
            /**
             * @brief Create the texture for the icon
             * 
             */
            void crt_tex();

            bool is_entered = false;//< Is mouse on the button?
            bool is_pressed = false;//< Is mouse pressed the button?

            //For control the button draw style
            bool draw_border_on_hover = true;//< Draw border when mouse on the button?
            bool draw_border = true;//< Draw border on normal status?
            
            //For RadioButton and CheckButton
            bool checked = false;
            bool checkable = true;

            u8string btext;//< Button text
            PixBuf   bicon;//< Button icon
            Texture  bicon_tex;//< Button icon's texture

            Signal<void()> clicked;
            Signal<void()> hovered;
        private:
            bool need_crt_tex = false;
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

            void draw(Renderer &,Uint32) override;
        protected:
            bool handle_mouse(MouseEvent &) override;
            void onleave() override;
            void onenter() override;
            //PixBuf  textbuf;
            //Texture texture;
            //Font    textfont;
    };
    /**
     * @brief RadioButton
     * 
     */
    class BTKAPI RadioButton:public AbstractButton{
        public:
            using Widget::set_rect;

            RadioButton();
            //FIXME MSVC could found vtable here
            RadioButton(u8string_view text);
            ~RadioButton();

            void draw(Renderer &,Uint32) override;
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
    /**
     * @brief CheckButton
     * 
     */
    class BTKAPI CheckButton:public AbstractButton{
        public:
            using Widget::set_rect;
            
            CheckButton();
            CheckButton(u8string_view text){
                btext = text;
            }
            ~CheckButton();
            void draw(Renderer &,Uint32) override;
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
