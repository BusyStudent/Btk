#if !defined(_BTK_BUTTON_HPP_)
#define _BTK_BUTTON_HPP_
#include "widget.hpp"
#include "pixels.hpp"
#include "font.hpp"
#include "defs.hpp"
namespace Btk{
    /**
     * @brief Basic button
     * 
     */
    class BTKAPI AbstructButton:public Widget{
        public:
            //Process event
            bool handle(Event &);
        protected:
            virtual void onclick(int x,int y) = 0;
            bool is_entered;//Is mouse on the button?
    };
    /**
     * @brief A simple pushbutton
     * 
     */
    class BTKAPI Button:public AbstructButton{
        public:
            void draw(Renderer &);
        protected:
            void onclick(int x,int y);
    };
};


#endif // _BTK_BUTTON_HPP_
