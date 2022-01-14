#if !defined(_BTK_SCROLLBAR_HPP_)
#define _BTK_SCROLLBAR_HPP_
#include "defs.hpp"
#include "pixels.hpp"
#include "widget.hpp"
namespace Btk{
    /**
     * @brief AbstractSlider
     * 
     */
    class BTKAPI AbstractSlider:public Widget{
        public:
            //Event handle
            bool handle_mouse(MouseEvent &) override;
            bool handle_drag(DragEvent &) override;
            bool handle_wheel(WheelEvent &) override;


            void set_rect(const Rect &r) override;
            void set_orientation(Orientation);
            void move_silder(float value);
            //Get signal
            Signal<void(float)> &signal_value_changed() noexcept{
                return _signal_change;
            }
        protected:
            //---------------//
            //  |Silder|     //
            //---Sildeable---//
            //The silder
            FRect sildeable_rect;
            FRect slider_rect;
            Color slider_color;

            float wheel_step = 1.0f;
            
            Orientation orientation;
            Signal<void(float)> _signal_change;

    };
    class BTKAPI SilderBar:public AbstractSlider{

    };
    class BTKAPI ScrollBar:public Widget{
        public:
            using Widget::set_rect;
            /**
             * @brief Construct a new Scroll Bar object
             * 
             * @param orientation The scrollbar's orientation
             */
            ScrollBar(Orientation orientation);
            ScrollBar(const ScrollBar &) = delete;
            ~ScrollBar();

            bool handle(Event &) override;
            void draw(Renderer&) override;
            /**
             * @brief Get the value of the scrollbar
             * 
             * @return 0 on min,100 on max
             */
            int value() const noexcept{
                return bar_value;
            }
            void set_value(int value);
            int move_slider(int x);
            
            void set_rect(const Rect &r) override;
            void set_parent(Widget *w) override;

            template<class Method,class TObject>
            void set_move_signal(Method&& method,TObject* object)
            {
                signal.connect(method,object);
            }
            Signal<void(int)> &signal_moved(){
                return signal;
            }
            
            bool handle_drag(DragEvent &) override;
            bool handle_wheel(WheelEvent &) override;
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
            // Rect bar_rect;
            //< The rect of the bar's range
            Rect bar_range;
            //< The value of the var,100 on max,0 on min
            int bar_value = 0;
            int max_bar_value = 100;
            //< When the flag is true,draw hight color
            bool actived = false;
            bool dragging = false;
            //When min value of the bar
            int min = 5;
            int max = 99;
            //The signal when the silder was moved
            Signal<void(int)> signal;
            //The silder
            Rect slider_rect;
            Color slider_color;
    };
}


#endif // _BTK_SCROLLBAR_HPP_
