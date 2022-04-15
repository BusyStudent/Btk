#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/scrollbar.hpp>

namespace Btk{
    // //TODO:? probably make a class for this
    ScrollBar::ScrollBar(Orientation ori){
        orientation = ori;
    }
    ScrollBar::~ScrollBar(){

    }
    FRect ScrollBar::content_rect() const noexcept{
        //Use fixed size hint if available
        FRect rect = rectangle<float>();
        FRect result = margin.apply(rect);
        if(enable_fixed_size){
            if(is_vertical()){
                //Find right of the content
                result.w = fixed_size_width;
                result.x = rect.x + rect.w - result.w;
            }
            else{
                //Find bottom of the content
                result.h = fixed_size_height;
                result.y = rect.y + rect.h - result.h;
            }
        }
        return result;
    }
    FRect ScrollBar::bar_rect() const noexcept{
        auto rect = content_rect();
        FRect result;

        if(is_vertical()){
            //Useable
            float w = rect.w - _long;
            //Calc the slider rect by value
            float h = (rect.h - _long) * (_value - _min_value) / (_max_value - _min_value);
            //Calc the slider rect by value
            result.x = rect.x;
            result.y = rect.y + rect.h - h - _long;
            result.w = w;
            result.h = _long;
        }
        else{
            //Useable
            float h = rect.h - _long;
            //Calc the slider rect by value
            float w = (rect.w - _long) * (_value - _min_value) / (_max_value - _min_value);
            //Calc the slider rect by value
            result.x = rect.x + rect.w - w - _long;
            result.y = rect.y;
            result.w = _long;
            result.h = h;
        }
        return result;
    }
    void ScrollBar::draw(Renderer &render,Uint32){
        draw_bounds();
        auto content = content_rect();
        auto bar = bar_rect();

        render.draw_box(content,theme().active.background);
        render.draw_rect(content,theme().active.border);

        render.draw_rect(bar,theme().active.button);
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