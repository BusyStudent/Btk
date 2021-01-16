#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include "rect.hpp"
#include "widget.hpp"
namespace Btk{
    class Layout:public Widget,Container{
        public:
            void draw(Renderer &render);
            bool handle(Event&) override;
            //update each widgets postions
            virtual void update() = 0;
        
    };
    class BoxLayout:public Layout{

    };
    class VBoxLayout:public BoxLayout{

    };
    class HBoxLayout:public BoxLayout{

    };
    class GridLayout:public Layout{

    };
};


#endif // _BTK_LAYOUT_HPP_
