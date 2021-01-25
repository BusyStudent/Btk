#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/scrollbar.hpp>

namespace Btk{
    //Unfinished yet
    /**
     * @brief The width of the var
     * 
     */
    constexpr int BarWidth = 5;
    constexpr int SingleStep = 4;
    ScrollBar::ScrollBar(Container &parent,Orientation orient):
        Widget(parent){

        orientation = orient;
        bar_color = {193,193,193};
        bar_hight_color = window()->theme.high_light;
    }
    ScrollBar::~ScrollBar(){}

    bool ScrollBar::handle(Event &event){
        switch(event.type()){
            case Event::SetRect:
                rect = event_cast<SetRectEvent&>(event).rect;
                if(orientation == Orientation::H){
                    int y = CalculateYByAlign(rect,BarWidth,Align::Center);
                    bar_rect = {rect.x,y,rect.w,BarWidth};
                }
                else{
                    int x = CalculateXByAlign(rect,BarWidth,Align::Center);
                    bar_rect = {x,rect.y,BarWidth,rect.w};
                }
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
        if(actived){
            render.rounded_box(bar_rect,3,bar_hight_color);
        }
        else{
            render.rounded_box(bar_rect,3,bar_color);
        }
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
        /*
        //Set bar's rect
        if(orientation == Orientation::H){
            //change the width
            bar_rect.w = int(float(rect.w) / 100 * value);
        }
        */
        redraw();
    }
}