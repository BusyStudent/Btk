#if !defined(_BTK_LAYOUT_HPP_)
#define _BTK_LAYOUT_HPP_
#include "rect.hpp"
#include "widget.hpp"
#include "container.hpp"

#include <map>

namespace Btk{
    class BTKAPI Layout:public Group{
        public:
            Layout();
            ~Layout();

            bool handle(Event &) override;
            void set_rect(const Rect &) override;

            //update each widgets postions
            virtual void update() = 0;
    };
    /**
     * @brief A Box Container
     * 
     */
    class BTKAPI BoxLayout:public Layout{
        public:
            enum Direction:Uint8{
                LeftToRight,//H
                RightToLeft,
                TopToBottom,//V
                BottomToTop
            };
        public:
            BoxLayout(Direction d);
            ~BoxLayout();

            void update() override;
            void draw(Renderer &) override;
            // bool add(Widget *w) override;
            void set_direction(Direction direction);
            Direction direction() const noexcept{
                return _direction;
            }
        private:
            bool is_dirty = true;
            bool lazy = true;

            Direction _direction;
            float spacing = 0;
    };
    class BTKAPI VBoxLayout:public BoxLayout{
        public:
            VBoxLayout();
            ~VBoxLayout();
    };
    class BTKAPI HBoxLayout:public BoxLayout{
        public:
            HBoxLayout();
            ~HBoxLayout();
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
