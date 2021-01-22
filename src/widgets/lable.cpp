#include "../build.hpp"

#include <Btk/impl/render.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/window.hpp>
#include <Btk/themes.hpp>
#include <Btk/lable.hpp>
namespace Btk{
    //Lable Impl
    
    Lable::Lable(Container &w){
        parent = &w;
        font_ = window()->default_font;
        //Set text color inherted at window
        text_color = window()->theme->text_color;
    }
    Lable::Lable(Container &w,std::string_view text){
        parent = &w;
        font_ = window()->default_font;
        //Set text color inherted at window
        text_color = window()->theme->text_color;
        text_ = text;
    }
    //Construct from posititon
    Lable::Lable(Container &wi,int x,int y,int w,int h){
        parent = &wi;
        font_ = window()->default_font;
        //Set text color inherted at window
        text_color = window()->theme->text_color;

        rect = {
            x,y,w,h
        };
    }
    Lable::~Lable(){

    }
    void Lable::draw(Renderer &render){
        if(text_.size() == 0){
            return;
        }
        if(texture.empty()){
            if(text_buf.empty()){
                text_buf = font_.render_blended(text_,text_color);
            }
            texture = render.create_from(text_buf);
        }
        //begin render
        //limit the position
        Rect text_rect;
        Rect cliprect = render.get_cliprect();

        //Calculate text postiton
        text_rect = CalculateRectByAlign(
            rect,
            text_buf->w,
            text_buf->h,
            v_align,
            h_align
        );
        
        render.set_cliprect(rect);
        render.copy(texture,nullptr,&text_rect);
        render.set_cliprect(cliprect);
    }
    void Lable::set_text(std::string_view text){
        text_ = text;
        texture  = nullptr;
        text_buf = nullptr;
        
        redraw();
    }
};