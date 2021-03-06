#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/window.hpp>
#include <Btk/render.hpp>
#include <Btk/button.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>
#include <Btk/font.hpp>
namespace Btk{
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
        Widget::set_parent(w);
        ptsize = theme().font_size();
    }
};
namespace Btk{
    Button::Button() = default;
    Button::Button(int x,int y,int w,int h){
        attr.user_rect = true;
        rect = {
            x,y,w,h
        };
    }
    Button::Button(u8string_view text){
        btext = text;
    }
    Button::~Button() = default;
    //draw button
    void Button::draw(Renderer &render){
        //Fist draw backgroud
        //Rect{rect.x,rect.y + 1,rect.w - 1,rect.h - 1}
        //It makes button look better
        Rect fixed_rect = {rect.x,rect.y + 1,rect.w - 1,rect.h - 2};
        Color bg;//< Background color
        Color boarder;//< Boarder color

        
        if(is_pressed){
            bg = theme()[Theme::Highlight];
        }
        else{
            bg = theme()[Theme::Button];
        }
        
        //second draw border
        if(is_entered){
            boarder = theme()[Theme::Highlight];
        }
        else{
            boarder = theme()[Theme::Border];
        }
        //Draw box
        render.begin_path();
        render.fill_color(bg);
        render.rounded_rect(fixed_rect,2);
        render.fill();
        //Render text
        if(btext.size() != 0){
            //has text
            render.save();
            
            #if 0
            if(texture.empty()){
                if(textbuf.empty()){
                    //render text
                    if(is_pressed){
                        textbuf = textfont.render_blended(btext,theme()->high_light_text);
                    }
                    else{
                        textbuf = textfont.render_blended(btext,theme()->text_color);
                    }
                }
                texture = render.create_from(textbuf);
            }
            auto pos = CalculateRectByAlign(rect,textbuf->w,textbuf->h,Align::Center,Align::Center);
            #endif
            render.intersest_scissor(rect);
            // render.copy(texture,nullptr,&pos);
            render.begin_path();
            render.text_size(ptsize);
            render.text_align(TextAlign::Center | TextAlign::Middle);

            if(is_pressed){
                render.fill_color(theme()[Theme::HighlightedText]);
            }
            else{
                render.fill_color(theme()[Theme::Text]);
            }
            //NOTE plus 2 to make it look better
            float x = float(fixed_rect.x) + float(fixed_rect.w) / 2 + 2;
            float y = float(fixed_rect.y) + float(fixed_rect.h) / 2 + 2;

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
        render.begin_path();
        render.stroke_color(boarder);
        render.rounded_rect(fixed_rect,2);
        render.stroke();
        
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
                clicked.emit();
            }
        }
        return event.accept();
    }
    void Button::onleave(){
        is_entered = false;
        is_pressed = false;
        redraw();
    }
    void Button::set_text(u8string_view text){
        btext = text;
        redraw();
    }
}
//RadioButton
namespace Btk{
    RadioButton::~RadioButton() = default;
    RadioButton::RadioButton() = default;
    void RadioButton::draw(Renderer &render){
        //Draw text
        if(not btext.empty()){
            render.begin_path();
            render.use_font(theme().font_name());
            render.text_size(theme().font_size());
            render.text_align(TextAlign::Middle);
            render.fill_color(theme()[Theme::Text]);
            render.text(text_center,btext);
            render.fill();
        }
        
        Color circle_c;
        Color circle_b;
        if(checked){
            circle_c = theme()[Theme::Highlight];
        }
        else{
            circle_c = theme()[Theme::Background];
        }
        if(is_entered){
            circle_b = theme()[Theme::Highlight];
        }
        else{
            circle_b = theme()[Theme::Border];
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
            signal_clicked();
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
    void CheckButton::draw(Renderer &render){
        if(not btext.empty()){
            //Draw the text
            render.begin_path();
            render.use_font(theme().font_name());
            render.text_size(theme().font_size());
            render.text_align(TextAlign::Middle);
            render.fill_color(theme()[Theme::Text]);
            render.text(text_center,btext);
            render.fill();
        }
        //Select color
        Color hight_light = theme()[Theme::Highlight];
        Color rect_boarder;

        if(is_entered){
            rect_boarder = hight_light;
        }
        else{
            //Normal boarder
            rect_boarder = theme()[Theme::Border];
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