#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/window.hpp>
#include <Btk/themes.hpp>
#include <Btk/render.hpp>
#include <Btk/label.hpp>

namespace Btk{
    //Label Impl
    
    Label::Label(){
        //parent = &w;
        //font_ = window()->font();
        //Set text color inherted at window

    }
    Label::Label(std::string_view text){
        //parent = &w;
        //font_ = window()->font();
        //Set text color inherted at window
        //text_color = window()->theme.text_color;
        text_ = text;

        //ptsize = window()->theme.font.ptsize();
    }
    //Construct from posititon
    Label::Label(int x,int y,int w,int h){
        //parent = &wi;
        //font_ = window()->font();
        //Set text color inherted at window
        //text_color = window()->theme.text_color;

        rect = {
            x,y,w,h
        };
        attr.user_rect = true;

        //ptsize = window()->theme.font.ptsize();
    }
    Label::~Label(){

    }
    void Label::draw(Renderer &render){
        if(text_.empty()){
            return;
        }
        #if 0
        if(texture.empty()){
            if(text_buf.empty()){
                text_buf = font_.render_blended(text_,text_color);
            }
            texture = render.create_from(text_buf);
        }
        #endif
        render.save();
        //begin render
        //limit the position
        //Rect text_rect;

        //Calculate text postiton
        #if 0
        text_rect = CalculateRectByAlign(
            rect,
            text_buf->w,
            text_buf->h,
            v_align,
            h_align
        );
        #endif
        render.intersest_scissor(rect);
        //render.copy(texture,nullptr,&text_rect);

        render.begin_path();
        render.fill_color(text_color);
        render.text_align(v_align,h_align);
        render.text_size(ptsize);
        render.text(
            float(rect.x),
            rect.y + float(rect.h) / 2,
            text_);
        
        render.fill_color(text_color);
        render.fill();
        
        render.restore();
    }
    void Label::set_text(std::string_view text){
        text_ = text;
        //texture  = nullptr;
        //text_buf = nullptr;
        
        redraw();
    }
    void Label::set_parent(Widget *w){
        Widget::set_parent(w);
        text_color = window_theme()[Theme::Text];
        ptsize = window_theme().font_size();
    }
};