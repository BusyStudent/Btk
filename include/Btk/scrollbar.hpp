#if !defined(_BTK_SCROLLBAR_HPP_)
#define _BTK_SCROLLBAR_HPP_
#include "defs.hpp"
#include "pixels.hpp"
#include "widget.hpp"
namespace Btk{
    #if 0
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
    #endif
    /**
     * @brief A SliderBar
     * 
     */
    class BTKAPI SliderBar:public Widget{
        public:
            SliderBar(Orientation orientation);
            ~SliderBar();

            void draw(Renderer &) override;
            //Event handle
            bool handle(Event &) override;
            bool handle_drag(DragEvent &) override;
            bool handle_mouse(MouseEvent &) override;
            bool handle_motion(MotionEvent &) override;
            bool handle_wheel(WheelEvent &) override;
            /**
             * @brief Set the value object
             * 
             * @param value 
             * @return Current value of
             */
            float set_value(float value);
            /**
             * @brief The value of the slider
             * @note The value you probably should round it
             * @return float 
             */
            float value() const noexcept{
                return _value;
            }
            float step() const noexcept{
                return _step;
            }
            void  set_step(float step){
                _step = step;
            }
            void  set_bar_radius(float radius){
                bar_radius = radius;
                redraw();
            }
            void  set_cirle_radius(float radius){
                circle_radius = radius;
                redraw();
            }
            /**
             * @brief Set the min max object(the max must be greater than min)
             * 
             * @param min 
             * @param max 
             */
            void  set_min_max(float min,float max);

            void  set_min_value(float min){
                set_min_max(min,_max_value);
            }
            void  set_max_value(float max){
                set_min_max(_min_value,max);
            }
            Signal<void(float)> &signal_value_changed() noexcept{
                return _signal_changed;
            }
            /**
             * @brief Set allow the drag to change the value
             * 
             * @param allow 
             */
            void set_allow_drag(bool allow){
                allow_mouse_drag = allow;
            }
            /**
             * @brief Set the allow wheel to change the value
             * 
             * @param allow 
             */
            void set_allow_wheel(bool allow){
                allow_wheel_scroll = allow;
            }
            /**
             * @brief Set the allow press to change the value
             * 
             * @param allow 
             */
            void set_allow_press(bool allow){
                allow_mouse_press = allow;
            }
        protected:
            //Utils
            /**
             * @brief The bar rectange  in the widget
             * 
             * @return FRect 
             */
            FRect  content_rect() const noexcept;
            /**
             * @brief The circle center of the bar
             * 
             * @return FPoint 
             */
            FPoint content_circle() const noexcept;

            bool is_vertical() const noexcept{
                return orientation == Vertical;
            }
        private:
            //Value for slider
            float _value = 0.0f;//< The value of the slider
            float _min_value = 0.0f;//< The minimum value of the slider
            float _max_value = 100.0f;//< The maximum value of the slider
            float _step = 1.0f;
            //Bar
            float bar_radius = 4.0f;

            //< The slider circle
            float circle_radius = 10.0f;

            //Status for hovered and pressed
            bool hovered = false;
            bool pressed = false;
            bool dragging = false;
            bool has_dragged = false;

            //Hint for the slider

            bool allow_mouse_press = true;
            bool allow_mouse_drag = true;
            bool allow_wheel_scroll = true;

            //< If the distance of dragging is too low, we don't consider it as dragging
            int sum_drag = 0;
            int sum_drag_max = 2;//< The value of sum_drag when we consider it as dragging
            
            Orientation orientation;
            Signal<void(float)> _signal_changed;

            FMargin margin = {
                2,
                2,
                2,
                2 //< Anti-aliasing make it fatter
            };
            //It think it is better to query margin from system

            //Fixed size hint
            //TODO : query the size hint from system
            //TODO : cached the size hint
            float fixed_size_width = 10;
            float fixed_size_height = 10;
            float fixed_size_circle_radius = 10.0f;
            float fixed_size_bar_radius = 4.0f;
            
            bool enable_fixed_size = true;
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
