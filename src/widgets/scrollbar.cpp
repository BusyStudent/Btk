#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/debug.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/scrollbar.hpp>

namespace Btk{
    //Unfinished yet
    /**
     * @brief The width of the var
     * 
     */
    constexpr int BarWidth = 10;
    constexpr int SingleStep = 4;
    int _x = 10,_y = 10;
    ScrollBar::ScrollBar(Orientation orient){
        attr.focus = FocusPolicy::Wheel;
        orientation = orient;
        bar_color = {200,200,200};
        bar_bg_color = {173,175,178};
        slider_color = {100,100,100};
    }
    ScrollBar::~ScrollBar(){}

    bool ScrollBar::handle(Event &event){
        switch(event.type()){
            case Event::SetRect:
                rect = event_cast<SetRectEvent&>(event).rect();
                //Set bar range
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
                return true;
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
            case Event::DragBegin:{
                event.accept();
                dragging = true;
                actived = true;
                return true;
            }
            case Event::DragEnd:{
                event.accept();
                dragging = false;
                actived = false;
                return true;
            }
            case Event::Wheel:{
                auto wheel = event_cast<WheelEvent&>(event);
                
                if(wheel.y > 0){
                    move_slider(SingleStep);
                }
                else{
                    move_slider(-SingleStep);
                }
                return true;
            }
            case Event::SetContainer:{
                event.accept();
                parent = event_cast<SetContainerEvent&>(event).container();
                window()->pixels_size(&_x,&_y);
                if(orientation == Orientation::H)set_rect(0,_y - 10,_x,10);
                else if(orientation == Orientation::V)set_rect(_x-10,0,10,_y);
                return true;
            }

            case Event::Drag :{
                DragEvent& e = event_cast<DragEvent&>(event);
                int x = e.xrel;
                int y = e.yrel;
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
            }
            default:
                return false;
        }
    }
    void ScrollBar::draw(Renderer &render){
        auto cliprect = render.get_cliprect();
        render.set_cliprect(rect);
        //draw background
        //Rect range = {bar_range.x,bar_range.y + 1,bar_range.w - 1,bar_range.h - 2}; 
        render.rounded_box(bar_range,1,bar_bg_color);
        // render.box(bar_rect,bar_color);
        // render.set_cliprect(cliprect);

        // //draw the slider
        // cliprect = render.get_cliprect();
        // render.set_cliprect(rect);
        render.box(slider_rect,slider_color);
        render.set_cliprect(cliprect);
    }
    void ScrollBar::set_value(int value){
        bar_value = 0;
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