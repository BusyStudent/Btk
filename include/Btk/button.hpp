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
    class BTKAPI AbstructButton:public Widget{
        public:
            //Process event
            bool handle(Event &);
        protected:
            virtual void onclick(const MouseEvent &) = 0;
            virtual void onenter();
            virtual void onleave();
            bool is_entered = false;//< Is mouse on the button?
            bool is_pressed = false;//< Is mouse pressed the button?
            Theme *theme = nullptr;//< current theme
            float ptsize = 0;//< fontsize
    };
    /**
     * @brief A simple pushbutton
     * 
     */
    class BTKAPI Button:public AbstructButton{
        public:
            Button();
            Button(std::string_view text);
            Button(int x,int y,int w,int h);
            ~Button();
            void draw(Renderer &);
            template<class ...T>
            void on_click(T &&...args){
                clicked.connect(std::forward<T>(args)...);
            };
            Signal<void()> &sig_click(){
                return clicked;
            };
            void set_text(std::string_view text);
            /**
             * @brief Get the button text
             * 
             * @return std::string_view 
             */
            std::string_view text() const{
                return btext;
            }
        protected:
            void onclick(const MouseEvent &);
            void onleave();

            Signal<void()> clicked;

            //Button text
            std::string btext;
            //PixBuf  textbuf;
            //Texture texture;
            //Font    textfont;
    };
};


#endif // _BTK_BUTTON_HPP_
