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
            bool handle(Event &);
            
            template<class ...T>
            void on_click(T &&...args){
                clicked.connect(std::forward<T>(args)...);
            }
            Signal<void()> &signal_clicked(){
                return clicked;
            }

        protected:
            virtual void set_parent(Widget *) override;
            virtual void onenter();
            virtual void onleave();
            bool is_entered = false;//< Is mouse on the button?
            bool is_pressed = false;//< Is mouse pressed the button?
            float ptsize = 0;//< fontsize
            
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
            void set_text(u8string_view text);
            /**
             * @brief Get the button text
             * 
             * @return u8string_view 
             */
            u8string_view text() const{
                return btext;
            }
        protected:
            bool handle_mouse(MouseEvent &) override;
            void onleave() override;


            //Button text
            u8string btext;
            //PixBuf  textbuf;
            //Texture texture;
            //Font    textfont;
    };
    class RadioButton:public AbstractButton{
        public:
            using Widget::set_rect;

            RadioButton();
            RadioButton(u8string_view text):RadioButton(){
                btext = text;
            }
            ~RadioButton();

            void draw(Renderer &) override;
            bool is_checked() const noexcept{
                return checked;
            }
            void set_rect(const Rect &r) override;
        protected:
            bool handle_motion(MotionEvent &event) override;
            bool handle_mouse(MouseEvent &event) override;
        private:
            bool checked = false;
            bool checkable = true;

            FPoint circle_center = {0,0};
            float  circle_r = 0;
            /**
             * @brief Text center
             * 
             */
            FPoint text_center;
            u8string btext;
    };
};


#endif // _BTK_BUTTON_HPP_
