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
    bool AbstructButton::handle(Event &event){
        switch(event.type()){
            case Event::Enter:{
                onenter();
                break;
            }
            case Event::Leave:{
                onleave();
                break;
            }
            case Event::SetRect:{
                //SetPositions
                rect = event_cast<SetRectEvent&>(event).rect();
                break;
            }
            case Event::Click:{
                //Click button
                auto &ev = event_cast<MouseEvent&>(event);
                onclick(ev);
                break;
            }
            case Event::SetContainer:{
                auto &ev = event_cast<SetContainerEvent&>(event);
                parent = ev.container();

                theme = &window()->theme;
                ptsize = window()->font().ptsize();
                break;
            }
            default:
                return false;
        }
        event.accept();
        return true;
    }
    void AbstructButton::onenter(){
        is_entered = true;

        redraw();
    }
    void AbstructButton::onleave(){
        is_entered = false;
        is_pressed = false;

        redraw();
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
            bg = theme->high_light;
        }
        else{
            bg = theme->button_color;
        }
        
        //second draw border
        if(is_entered){
            boarder = theme->high_light;
        }
        else{
            boarder = theme->border_color;
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
            auto cliprect = render.get_cliprect();
            render.set_cliprect(rect);
            // render.copy(texture,nullptr,&pos);
            render.begin_path();
            render.text_size(ptsize);
            render.text_align(TextAlign::Center | TextAlign::Middle);

            if(is_pressed){
                render.fill_color(theme->high_light_text);
            }
            else{
                render.fill_color(theme->text_color);
            }
            //NOTE plus 2 to make it look better
            float x = float(fixed_rect.x) + float(fixed_rect.w) / 2 + 2;
            float y = float(fixed_rect.y) + float(fixed_rect.h) / 2 + 2;

            BTK_LOGINFO("Text's x=%f y=%f",x,y);

            render.text(
                x,
                y,
                btext
            );


            render.fill();

            render.set_cliprect(cliprect);
            
            render.restore();
        }
        //draw the boarder
        render.begin_path();
        render.stroke_color(boarder);
        render.rounded_rect(fixed_rect,2);
        render.stroke();
        
    }
    void Button::onclick(const MouseEvent &event){
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