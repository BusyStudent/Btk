#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/window.hpp>
#include <Btk/render.hpp>
#include <Btk/button.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    AbstractButton::AbstractButton(){

    }
    bool AbstractButton::handle(Event &event){
        //The Widget had already processd it
        if(Widget::handle(event)){
            return true;
        }
        switch(event.type()){
            case Event::Enter:{
                onenter();
                break;
            }
            case Event::Leave:{
                onleave();
                break;
            }
            default:
                return false;
        }
        event.accept();
        return true;
    }
    void AbstractButton::onenter(){
        is_entered = true;

        redraw();
    }
    void AbstractButton::onleave(){
        is_entered = false;
        is_pressed = false;

        redraw();
    }
    void AbstractButton::set_parent(Widget *w){
        if(w == nullptr){
            //Clear texture
            bicon_tex.clear();
        }
        Widget::set_parent(w);
    }
    void AbstractButton::set_text(u8string_view text){
        btext = text;
        redraw();
    }
    void AbstractButton::set_icon(PixBufRef icon){
        bicon = icon;

        //Create texture
        if(window() != nullptr and IsMainThread()){
            bicon_tex = renderer()->create_from(bicon);
        }
        else{
            need_crt_tex = true;
        }

        redraw();
    }
    void AbstractButton::crt_tex(){
        if(need_crt_tex){
            bicon_tex = renderer()->create_from(bicon);
            need_crt_tex = false;
        }
    }
};
namespace Btk{
    Button::Button() = default;
    Button::Button(int x,int y,int w,int h){
        rect = {
            x,y,w,h
        };
    }
    Button::Button(u8string_view text){
        btext = text;
    }
    Button::~Button() = default;
    //draw button
    void Button::draw(Renderer &render,Uint32){
        render.save();
        render.set_antialias(false);
        //Fist draw backgroud
        //Rect{rect.x,rect.y + 1,rect.w - 1,rect.h - 1}
        //It makes button look better
        FRect fixed_rect = rectangle<float>();
        const Brush *bg;//< Background color
        const Brush *boarder;//< Boarder color

        fixed_rect.x += 1;
        fixed_rect.y += 1;
        fixed_rect.w -= 1;
        fixed_rect.h -= 1;
        
        if(is_pressed){
            bg = &theme().active.highlight;
        }
        else{
            bg = &theme().active.button;
        }
        
        //second draw border
        if(is_entered){
            boarder = &theme().active.highlight;
        }
        else{
            boarder = &theme().active.border;
        }
        //Draw box
        render.draw_rounded_box(fixed_rect,theme().button_radius,*bg);
        //Render text
        if(btext.size() != 0){
            //has text
            render.save();
            
            render.intersest_scissor(rect);
            // render.copy(texture,nullptr,&pos);
            render.begin_path();
            render.use_font(font());
            render.text_align(TextAlign::Center | TextAlign::Middle);

            if(is_pressed){
                render.fill_color(theme().active.highlight_text);
            }
            else{
                render.fill_color(theme().active.text);
            }
            //NOTE plus 2 to make it look better
            float x = fixed_rect.hcenter() + 2;
            float y = fixed_rect.vcenter() + 2;

            //BTK_LOGINFO("Text's x=%f y=%f",x,y);

            render.text(
                x,
                y,
                btext
            );


            render.fill();
            
            render.restore();
        }
        //draw the boarder
        if(draw_border and not is_entered){
            render.draw_rounded_rect(fixed_rect,theme().button_radius,*boarder);
        }
        else if(draw_border_on_hover and is_entered){
            render.draw_rounded_rect(fixed_rect,theme().button_radius,*boarder);
        }
        render.restore();
    }
    bool Button::handle_mouse(MouseEvent &event){
        if(event.is_pressed() and event.button.is_left()){
            BTK_LOGINFO("This button is clicked %p",this);
            is_pressed = true;
            //has text 
            //render text
            redraw();
        }
        else if(event.button.is_left() and is_pressed){
            //release the button
            //The button was clicked 
            is_pressed = false;
            //render text
            redraw();
            if(not clicked.empty()){
                clicked.defer_emit();
            }
        }
        return event.accept();
    }
    void Button::onleave(){
        is_entered = false;
        is_pressed = false;
        redraw();
    }
    void Button::onenter(){
        is_entered = true;
        hovered();
        redraw();
    }
}
//RadioButton
namespace Btk{
    RadioButton::~RadioButton() = default;
    RadioButton::RadioButton() = default;
    RadioButton::RadioButton(u8string_view t){
        btext = t;
    }
    void RadioButton::draw(Renderer &render,Uint32){
        //Draw text
        if(not btext.empty()){
            render.use_font(font());
            render.text_align(TextAlign::Middle);
            render.draw_text(
                text_center.x,
                text_center.y,
                btext,
                theme().active.text
            );
        }
        
        Color circle_c;
        Color circle_b;
        if(checked){
            circle_c = theme().active.highlight;
        }
        else{
            circle_c = theme().active.background;
        }
        if(is_entered){
            circle_b = theme().active.highlight;
        }
        else{
            circle_b = theme().active.border;
        }
        //Make circle
        render.fill_circle(circle_center,circle_r - 2,circle_c);
        //Make boarder
        render.draw_circle(circle_center,circle_r,circle_b);
    }
    void RadioButton::set_rect(const Rect &r){
        Widget::set_rect(r);

        circle_r = rectangle<float>().h / 2;
        
        circle_center.y = float(rect.y) + float(rect.h) / 2;
        circle_center.x = rect.x + circle_r;

        //Calc the text center
        text_center = circle_center;

        text_center.x += circle_r;
        //Text Begin here
        //----
        //-()- Text
        //----
        text_center.x += 2;
        text_center.y += 2;
    }
    bool RadioButton::handle_motion(MotionEvent &event){
        //Check the status and redraw
        if(PointInCircle(circle_center,circle_r,event.position())){
            //Check in the center
            if(not is_entered){
                is_entered = true;
                redraw();
            }
        }
        else{
            if(is_entered){
                is_entered = false;
                redraw();
            }
        }
        return event.accept();
    }
    bool RadioButton::handle_mouse(MouseEvent &event){
        //Click in circle
        if(event.is_released() or (not event.button.is_left())){
            //The button is not left
            return event.reject();
        }
        if(PointInCircle(circle_center,circle_r,event.position())){
            //Check in the center
            if(checkable){
                checked = ! checked;
                redraw();
            }
            signal_clicked().defer_emit();
        }
        return event.accept();
    }
}
namespace Btk{
    //TODO It has not finished yet
    CheckButton::CheckButton() = default;
    CheckButton::~CheckButton() = default;

    void CheckButton::set_rect(const Rect &r){
        //Calc the rect
        Widget::set_rect(r);

    }
    void CheckButton::draw(Renderer &render,Uint32){
        if(not btext.empty()){
            //Draw the text
            render.begin_path();
            render.use_font(theme().font);
            render.text_align(TextAlign::Middle);
            render.fill_color(theme().active.text);
            render.text(text_center,btext);
            render.fill();
        }
        //Select color
        Color hight_light = theme().active.highlight;
        Color rect_boarder;

        if(is_entered){
            rect_boarder = hight_light;
        }
        else{
            //Normal boarder
            rect_boarder = theme().active.border;
        }
        //Draw rect
        render.draw_rect(check_rect,rect_boarder);
        //Do we need to draw the 
        
    }
    bool CheckButton::handle_motion(MotionEvent &event){
        //Check the status and redraw
        if(check_rect.has_point(event.position())){
            //Check in the center
            if(not is_entered){
                is_entered = true;
                redraw();
            }
        }
        else{
            if(is_entered){
                is_entered = false;
                redraw();
            }
        }
        return event.accept();
    }
    bool CheckButton::handle_mouse(MouseEvent &event){
        if(event.is_released() or (not event.button.is_left())){
            //The button is not left
            return event.reject();
        }
        if(check_rect.has_point(event.position())){
            //Check in the center
            if(checkable){
                checked = ! checked;
                redraw();
            }
            signal_clicked();
        }
        return event.accept();
    }
}