#include "../build.hpp"

#include <Btk/impl/render.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/widget.hpp>
namespace Btk{
    Line::Line(Orientation orient){
        orientation = orient;
    }
    Line::Line(int x,int y,int w,int h,Orientation orient):
        Line(orient){
        
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;

        attr.user_rect = true;
    }
    Line::~Line(){}

    void Line::draw(Renderer &render){
        render.begin_path();
        render.stroke_color(0,0,0,255);

        if(orientation == Orientation::H){
            //H
            float y = rect.y + rect.h / 2;
            //render.line();
            render.move_to(rect.x,y);
            render.line_to(rect.x + rect.w,y);
        }
        else{
            //V
            float x = rect.x + rect.w / 2;
            render.move_to(x,rect.y);
            render.line_to(x,rect.y + rect.h);
        }
        render.stroke();
    }
};