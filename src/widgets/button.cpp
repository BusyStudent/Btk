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
        using Type = Event::Type;
        switch(event.type()){
            case Type::Enter:{
                onenter();
                break;
            }
            case Type::Leave:{
                onleave();
                break;
            }
            case Type::SetRect:{
                //SetPositions
                rect = event_cast<SetRectEvent&>(event).rect;
                break;
            }
            case Type::Click:{
                //Click button
                auto &ev = event_cast<MouseEvent&>(event);
                onclick(ev);
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
        win->draw();
    }
    void AbstructButton::onleave(){
        is_entered = false;
        is_pressed = false;
        win->draw();
    }
};
namespace Btk{
    Button::Button(Window &w){
        BTK_ASSERT(&w != nullptr);
        win = &w;
        //Set theme
        theme     = w.impl()->theme;
        textfont  = w.impl()->default_font;
        is_entered = false;
        is_pressed = false;
    }
    Button::Button(Window &wi,int x,int y,int w,int h){
        BTK_ASSERT(&wi != nullptr);
        win = &wi;
        //Set theme
        theme     = wi.impl()->theme;
        textfont  = wi.impl()->default_font;
        is_entered = false;
        is_pressed = false;

        attr.user_rect = true;
        rect = {
            x,y,w,h
        };
    }
    Button::Button(Window &w,std::string_view text):btext(text){
        BTK_ASSERT(&w != nullptr);
        win = &w;
        //Set theme
        theme     = w.impl()->theme;
        textfont  = w.impl()->default_font;
        is_entered = false;
        is_pressed = false;

        textbuf = textfont.render_blended(text,theme->text_color);
    }
    Button::~Button(){}
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
            bg = theme->background_color;
        }
        
        //second draw border
        if(is_entered){
            boarder = theme->high_light;
        }
        else{
            boarder = theme->border_color;
        }
        //Draw box
        render.rounded_box(fixed_rect,2,bg);
        //Render text
        if(btext.size() != 0){
            //has text
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
            auto cliprect = render.get_cliprect();
            render.set_cliprect(rect);
            render.copy(texture,nullptr,&pos);
            render.set_cliprect(cliprect);
        }
        //draw the boarder
        render.rounded_rect(fixed_rect,2,boarder);
    }
    void Button::onclick(const MouseEvent &event){
        if(event.is_pressed() and event.button.is_left()){
            BTK_LOGINFO("This button is clicked %p",this);
            is_pressed = true;
            //has text 
            if(btext.size() != 0){
                textbuf = textfont.render_blended(btext,theme->high_light_text);
                texture = nullptr;
            }
            //render text
            win->draw();
        }
        else if(event.button.is_left() and is_pressed){
            //release the button
            //The button was clicked 
            is_pressed = false;
            //render text
            if(btext.size() != 0){
                textbuf = textfont.render_blended(btext,theme->text_color);
                texture = nullptr;
            }
            win->draw();
            if(not clicked.empty()){
                clicked.emit();
            }
        }
    }
    void Button::onleave(){
        if(btext.size() != 0){
            //reset text color
            textbuf = textfont.render_blended(btext,theme->text_color);
            texture = nullptr;
        }
        is_entered = false;
        is_pressed = false;
        win->draw();
    }
    void Button::set_text(std::string_view text){
        btext = text;
        win->draw();
    }
};