#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include <map>
#include "rect.hpp"
#include "widget.hpp"
namespace Btk{
    class Layout:public Widget,protected Container{
        public:
            virtual void draw(Renderer &render) final;
            virtual bool handle(Event&) final;
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
        private:

    };
};


#endif // _BTK_LAYOUT_HPP_
