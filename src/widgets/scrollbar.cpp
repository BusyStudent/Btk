#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/scrollbar.hpp>

namespace Btk{
    //Unfinished yet
    /**
     * @brief The width of the var
     * 
     */
    constexpr int BarWidth = 10;
    constexpr int SingleStep = 4;
    //...
    static thread_local int _x = 10;
    static thread_local int _y = 10;
    
    ScrollBar::ScrollBar(Orientation orient){
        attr.focus = FocusPolicy::Wheel;
        orientation = orient;
        bar_color = {200,200,200};
        bar_bg_color = {173,175,178};
        slider_color = {100,100,100};

        set_value(0);
    }
    ScrollBar::~ScrollBar(){}
    void ScrollBar::set_rect(const Rect &r){
        Widget::set_rect(r);
        if(orientation == Orientation::H){
            int y = CalculateYByAlign(rect,rect.h,Align::Center);
            bar_range = {rect.x,y,rect.w,rect.h};
        }
        else{
            int x = CalculateXByAlign(rect,rect.w,Align::Center);
            bar_range = {x,rect.y,rect.w,rect.h};
        }
        set_value(100);
        BTK_LOGINFO("bar_range={%d,%d,%d,%d}",bar_range.x,bar_range.y,bar_range.w,bar_range.h);
        //update bar_rect
        redraw();
    }
    bool ScrollBar::handle(Event &event){
        if(Widget::handle(event)){
            return true;
        }
        switch(event.type()){
            case Event::Enter:{
                BTK_LOGINFO("Enter scroll bar %p",this);
                event.accept();
                actived = true;
                redraw();
                return true;
            }
            case Event::Leave:{
                BTK_LOGINFO("Leave scroll bar %p",this);
                event.accept();
                if(not dragging){
                    actived = false;
                }
                
                redraw();
                return true;
            }
            default:
                return false;
        }
    }
    bool ScrollBar::handle_wheel(WheelEvent &event){
        if(event.y < 0){
            move_slider(SingleStep);
        }
        else{
            move_slider(-SingleStep);
        }
        return event.accept();
    }
    bool ScrollBar::handle_drag(DragEvent &event){
        switch(event.type()){
            case Event::DragBegin:{
                dragging = true;
                actived = true;
                break;
            }
            case Event::DragEnd:{
                event.accept();
                dragging = false;
                actived = false;
                break;
            }
            case Event::Drag :{
                int x = event.xrel;
                int y = event.yrel;
                if(orientation == Orientation::H){
                    //Is Horizontal
                    move_slider(x);
                }
                else{
                    /**
                     * ----
                     * |  |
                     * |  |
                     * |  |
                     * |  |
                     * ----
                     */
                    move_slider(y);
                }
                // BTK_LOGINFO("The Bar_value={%d}\nThe rel={%d,%d}",this->bar_value,x,y);
                break;
            }
            default:{}
        }
        return event.accept();
    }
    void ScrollBar::draw(Renderer &render,Uint32){
        render.save();
        render.intersest_scissor(rect);
        //draw background
        //Rect range = {bar_range.x,bar_range.y + 1,bar_range.w - 1,bar_range.h - 2}; 
        render.draw_rounded_box(bar_range,1,bar_bg_color);
        // render.box(bar_rect,bar_color);
        // render.set_cliprect(cliprect);

        // //draw the slider
        // cliprect = render.get_cliprect();
        // render.set_cliprect(rect);
        render.draw_box(slider_rect,slider_color);
        render.restore();
    }
    void ScrollBar::set_value(int value){
        // bar_value = 0;
        if(value <= 1){
            max_bar_value = 1;
        }
        else if(value >= 100){
            max_bar_value = 100;
        }
        else{
            max_bar_value = value;
        }
        if(not bar_range.empty()){
            /**---------
             * |       |
             * ---------
             */
            if(orientation == Orientation::H){
                //Is Horizontal
                // bar_rect.w = _x;
                // bar_rect.x = bar_range.x;

                // bar_rect.y = bar_range.y;
                // bar_rect.h = bar_range.h;

                slider_rect.w = ((float(bar_range.w)) / max_bar_value);
                slider_rect.x = (((bar_range.w) / (max_bar_value)) * bar_value);

                slider_rect.y = bar_range.y;
                slider_rect.h = bar_range.h;
            }
            else{
                /**
                 * ----
                 * |  |
                 * |  |
                 * |  |
                 * |  |
                 * ----
                 */
                // bar_rect.x = bar_range.x;
                // bar_rect.w = bar_range.w;

                // bar_rect.h = (float(bar_range.h)) / 100 * (bar_value + 1);
                // bar_rect.y = bar_range.y + bar_rect.h;
                slider_rect.w = bar_range.w;
                slider_rect.x = bar_range.x;
               
                slider_rect.h = (float(bar_range.h)) / max_bar_value;
                slider_rect.y =  (((bar_range.h)/ (max_bar_value)) * (bar_value));
            }
        }
        BTK_LOGINFO("bar_range={%d,%d,%d,%d}",bar_range.x,bar_range.y,bar_range.w,bar_range.h);
        BTK_LOGINFO("ScrollBar %p slider_rect={%d,%d,%d,%d}",
            this,
            slider_rect.x,
            slider_rect.y,
            slider_rect.w,
            slider_rect.h
        );
        redraw();
    }
    void ScrollBar::set_parent(Widget *w){
        Widget::set_parent(w);
        window()->pixels_size(&_x,&_y);
        if(orientation == Orientation::H)set_rect(0,_y - 10,_x,10);
        else if(orientation == Orientation::V)set_rect(_x-10,0,10,_y);

    }
    int ScrollBar::move_slider(int x)
    {
        if(orientation == Orientation::H)
        {
            slider_rect.x += x;
            if(slider_rect.x < bar_range.x) slider_rect.x = bar_range.x;
            else if(slider_rect.x > bar_range.w - slider_rect.w) slider_rect.x = bar_range.w - slider_rect.w;
            bar_value = max_bar_value*float(slider_rect.x)/(bar_range.w - slider_rect.w);
        }
        else
        {
            slider_rect.y += x;
            if(slider_rect.y < bar_range.y)slider_rect.y = bar_range.y;
            else if(slider_rect.y > bar_range.h - slider_rect.h) slider_rect.y = bar_range.h - slider_rect.h;
            bar_value = max_bar_value*float(slider_rect.y)/(bar_range.h - slider_rect.h);
        }
        redraw();
        BTK_LOGINFO("bar_value={%d}",bar_value);
        signal(bar_value);
        return bar_value;
    }
}

namespace Btk{
    //TODO AbstractSilder

    SliderBar::SliderBar(Orientation ori){
        orientation = ori;
    }
    SliderBar::~SliderBar(){

    }
    FRect SliderBar::content_rect() const noexcept{
        //Use fixed size hint if available
        FRect rect = rectangle<float>();
        FRect result = margin.apply(rect);
        if(enable_fixed_size){
            //Fixed size,make it center
            auto c = result.center();
            if(is_vertical()){
                //Calc of the rect depend the center
                result.w = fixed_size_width;
                result.x = c.x - fixed_size_width / 2;
            }
            else{
                result.h = fixed_size_height;
                result.y = c.y - fixed_size_height / 2;
            }
        }
        return result;
    }
    FPoint SliderBar::content_circle() const noexcept{
        FRect  rect = content_rect();
        FPoint circle;

        if(is_vertical()){
            circle.y = rect.y + rect.h / (_max_value - _min_value) * (_value - _min_value);
            circle.x = rect.x + rect.w / 2;
            //Calc the circle position by value
        }
        else{
            circle.x = rect.x + rect.w / (_max_value - _min_value) * (_value - _min_value);
            circle.y = rect.y + rect.h / 2;
            //Calc the circle position by value
        }

        return circle;
    }
    bool  SliderBar::handle(Event &event){
        if(Widget::handle(event)){
            return true;
        }
        if(event.type()  == Event::Leave){
            //< Assume the mouse is out of the widget
            if(hovered or pressed and not dragging){
                hovered = false;
                pressed = false;
                redraw();
            }
            return event.accept();
        }
        return false;
    }
    bool  SliderBar::handle_motion(MotionEvent &event){
        bool in_circle = PointInCircle(event.position(),circle_radius,content_circle());
        if(in_circle != hovered){
            //Status changed
            hovered = in_circle;
            redraw();
        }
        event.accept();
        return true;
    }
    bool  SliderBar::handle_mouse(MouseEvent &event){
        if(event.is_pressed() and hovered){
            pressed = true;
            redraw();
        }
        else if(event.is_released()){
            pressed = false;
            if(not has_dragged and allow_mouse_press){
                //Just a click
                //Calc the value by the position
                auto rect = content_rect();
                float new_value;
                if(is_vertical()){
                    new_value = _min_value + (_max_value - _min_value) * (event.y - rect.y) / rect.h;
                }
                else{
                    new_value = _min_value + (_max_value - _min_value) * (event.x - rect.x) / rect.w;
                }
                set_value(new_value);
            }
            has_dragged = false;
            redraw();
        }
        return true;
    }
    bool  SliderBar::handle_drag(DragEvent &event){
        switch(event.type()){
            case Event::DragBegin:
                dragging = true;
                break;
            case Event::DragEnd:
                dragging = false;
                has_dragged = sum_drag >= sum_drag_max;
                if(not rectangle().has_point(event.position())){
                    hovered = false;
                    pressed = false;
                }
                sum_drag = 0;
                redraw();
                break;
            case Event::Drag:
                //Calc the value by the relative position
                auto rect = content_rect();
                float new_value;
                if(is_vertical()){
                    new_value = _value + (_max_value - _min_value) * (event.yrel) / rect.h;
                    sum_drag += std::abs(event.yrel);
                }
                else{
                    new_value = _value + (_max_value - _min_value) * (event.xrel) / rect.w;
                    sum_drag += std::abs(event.yrel);
                }
                if(allow_mouse_drag){
                    set_value(new_value);
                }
                break;
        }
        return event.accept();
    }
    bool  SliderBar::handle_wheel(WheelEvent &event){
        if(not allow_wheel_scroll){
            return event.reject();
        }
        BTK_LOGINFO("[SliderBar] WheelEvent");
        if(is_vertical()){
            if(event.y > 0){
                set_value(_value + _step);
            }
            else{
                set_value(_value - _step);
            }
        }
        else{
            if(event.y < 0){
                set_value(_value + _step);
            }
            else{
                set_value(_value - _step);
            }
        }
        redraw();
        return event.accept();
    }
    void  SliderBar::draw(Renderer &r,Uint32){
        auto rect = content_rect();
        auto circle = content_circle();
        //Draw background
        r.draw_rounded_box(rect,bar_radius,theme().active.background);
        r.draw_rounded_rect(rect,bar_radius,theme().active.border);
        //Calc the slider circle position by value

        //Fill the bar by value
        if(_value == _min_value){
            //Donothing
        }
        else if(is_vertical()){
            //Use scissor to drop pixels
            r.save();
            r.intersest_scissor(
                rect.x,
                rect.y,
                rect.w,
                rect.h / (_max_value - _min_value) * (_value - _min_value)
            );
            r.draw_rounded_box(rect,bar_radius,theme().active.highlight);
            r.restore();
        }
        else{
            r.save();
            r.intersest_scissor(
                rect.x,
                rect.y,
                rect.w / (_max_value - _min_value) * (_value - _min_value),
                rect.h
            );
            r.draw_rounded_box(rect,bar_radius,theme().active.highlight);
            r.restore();
        }
        //Cirlce done

        //Process circle content
        if(pressed){
            r.fill_circle(circle,circle_radius,theme().active.highlight);
        }
        else{
            r.fill_circle(circle,circle_radius,theme().active.button);
        }

        //Process border
        if(hovered){
            r.draw_circle(circle,circle_radius,theme().active.highlight);
        }
        else{
            r.draw_circle(circle,circle_radius,theme().active.border);
        }
        //Draw done
    }
    float SliderBar::set_value(float v){
        float n = clamp(v,_min_value,_max_value);
        if(n == _value){
            return _value;
        }
        _value = n;

        redraw();
        //Leet the listener known
        _signal_changed(_value);
        return _value;
    }
    void  SliderBar::set_min_max(float min,float max){
        BTK_ASSERT(max > min);

        _min_value = clamp(min,0.0f,max);
        _max_value = max;
        set_value(_value);
    }
}