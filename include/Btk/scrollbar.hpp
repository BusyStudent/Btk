#if !defined(_BTK_SCROLLBAR_HPP_)
#define _BTK_SCROLLBAR_HPP_
#include "defs.hpp"
#include "pixels.hpp"
#include "widget.hpp"
namespace Btk{
    class BTKAPI ScrollBar:public Widget{
        public:
            /**
             * @brief Construct a new Scroll Bar object
             * 
             * @param parent The scrollbar's master
             * @param orientation The scrollbar's orientation
             */
            ScrollBar(Container &,Orientation orientation);
            ScrollBar(const ScrollBar &) = delete;
            ~ScrollBar();

            bool handle(Event &);
            void draw(Renderer&);
            /**
             * @brief Get the value of the scrollbar
             * 
             * @return 0 on min,100 on max
             */
            int value() const noexcept{
                return bar_value;
            }
            void set_value(int value);
        private:
            /*
            *|--Widget's rect-|
            *|                |
            *|----bar_range---|
            *|  ||bar_rect||  |
            *|----------------|
            *|                |
            *|----------------|
            */
            Orientation orientation;
            //<The color of the scroll bar
            Color bar_color;
            //<The color when user enter the bar's position
            Color bar_enter_color;
            //<The color when user press or drag the bar
            Color bar_pressed_color;
            //<Bar's background
            Color bar_bg_color;
            //< The rect of the bar
            Rect bar_rect;
            //< The rect of the bar's range
            Rect bar_range;
            //< The value of the var,100 on max,0 on min
            int bar_value = 0;
            //< When the flag is true,draw hight color
            bool actived = false;
            bool dragging = false;
            //When min value of the bar
            int min = 5;
            int max = 99;
    };
}


#endif // _BTK_SCROLLBAR_HPP_
