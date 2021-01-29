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
    constexpr int BarWidth = 6;
    constexpr int SingleStep = 4;
    ScrollBar::ScrollBar(Container &parent,Orientation orient):
        Widget(parent){

        orientation = orient;
        bar_color = {193,193,193};
        bar_bg_color = {173,175,178};
    }
    ScrollBar::~ScrollBar(){}

    bool ScrollBar::handle(Event &event){
        switch(event.type()){
            case Event::SetRect:
                rect = event_cast<SetRectEvent&>(event).rect;
                //Set bar range
                if(orientation == Orientation::H){
                    int y = CalculateYByAlign(rect,BarWidth,Align::Center);
                    bar_range = {rect.x + 2,y,rect.w - 2,BarWidth};
                }
                else{
                    int x = CalculateXByAlign(rect,BarWidth,Align::Center);
                    bar_range = {x,rect.y,BarWidth,rect.w};
                }
                BTK_LOGINFO("bar_range={%d,%d,%d,%d}",bar_range.x,bar_range.y,bar_range.w,bar_range.h);
                //update bar_rect
                set_value(bar_value);
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
                if(wheel.x > 0){
                    set_value(bar_value + SingleStep);
                }
                else{
                    set_value(bar_value - SingleStep);
                }
                return true;
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
        render.box(bar_rect,bar_color);
        render.set_cliprect(cliprect);
    }
    void ScrollBar::set_value(int value){
        if(value <= 0){
            bar_value = 0;
        }
        else if(value >= 100){
            bar_value = 100;
        }
        else{
            bar_value = value;
        }
        if(not bar_range.empty()){
            /**---------
             * |       |
             * ---------
             */
            if(orientation == Orientation::H){
                //Is Horizontal
                bar_rect.w = (float(bar_range.w)) / 100 * (bar_value + 1);
                bar_rect.x = bar_range.x + bar_rect.w;

                bar_rect.y = bar_range.y;
                bar_rect.h = bar_range.h;
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
                bar_rect.x = bar_range.x;
                bar_rect.w = bar_range.w;

                bar_rect.h = (float(bar_range.h)) / 100 * (bar_value + 1);
                bar_rect.y = bar_range.y + bar_rect.h;
            }
        }
        BTK_LOGINFO("ScrollBar %p bar_rect={%d,%d,%d,%d}",
            this,
            bar_rect.x,
            bar_rect.y,
            bar_rect.w,
            bar_rect.h
        );
        redraw();
    }
}