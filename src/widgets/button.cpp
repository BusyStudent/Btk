#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/window.hpp>
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
        theme = window_theme();
        ptsize = theme.font_size();
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
    Button::Button(std::string_view text):btext(text){}
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
            bg = theme[Theme::Highlight];
        }
        else{
            bg = theme[Theme::Button];
        }
        
        //second draw border
        if(is_entered){
            boarder = theme[Theme::Highlight];
        }
        else{
            boarder = theme[Theme::Border];
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
                        textbuf = textfont.render_blended(btext,theme->high_light_text);
                    }
                    else{
                        textbuf = textfont.render_blended(btext,theme->text_color);
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
                render.fill_color(theme[Theme::HighlightedText]);
            }
            else{
                render.fill_color(theme[Theme::Text]);
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
    bool Button::handle_click(MouseEvent &event){
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
    void Button::set_text(std::string_view text){
        btext = text;
        redraw();
    }
}
//RadioButton
namespace Btk{
    RadioButton::~RadioButton() = default;
    RadioButton::RadioButton() = default;
    void RadioButton::draw(Renderer &render){
        //...
    }
}