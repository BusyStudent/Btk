#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/detail/utils.hpp>
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
    Label::Label(u8string_view text){
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
    void Label::draw(Renderer &render,Uint32){
        if(text_.empty()){
            return;
        }
        render.save();
        //begin render
        //limit the position
        //Rect text_rect;

        render.intersest_scissor(rect);
        //render.copy(texture,nullptr,&text_rect);

        render.begin_path();
        render.fill_color(text_color);
        render.text_align(align);
        render.text_size(ptsize);
        render.text(
            float(rect.x),
            rect.y + float(rect.h) / 2,
            text_);
        
        render.fill();
        
        render.restore();
    }
    void Label::set_text(u8string_view text){
        text_ = text;
        //texture  = nullptr;
        //text_buf = nullptr;
        
        redraw();
    }
    void Label::set_parent(Widget *w){
        Widget::set_parent(w);
        text_color = theme().active.text;
        ptsize = theme().font.ptsize();
    }
};