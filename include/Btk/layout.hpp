#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include <map>
#include "rect.hpp"
#include "widget.hpp"
namespace Btk{
    class Layout:public Widget{
        public:
            virtual void draw(Renderer &render) final;
            //update each widgets postions
            virtual void update() = 0;
        protected:

    };
    class BTKAPI BoxLayout:public Layout{

    };
    class BTKAPI VBoxLayout:public BoxLayout{

    };
    class BTKAPI HBoxLayout:public BoxLayout{

    };
    class BTKAPI GridLayout:public Layout{
        public:
            GridLayout();
            GridLayout(const GridLayout &) = delete;
            ~GridLayout();

            void update();
            bool add_row(Widget *widget);
            void add();
        private:
            
    };
};


#endif // _BTK_LAYOUT_HPP_
