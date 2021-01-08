#include "../build.hpp"

#include <Btk/impl/render.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/widget.hpp>
namespace Btk{
    Line::Line(Window &w,Orientation orient){
        win = &w;
        orientation = orient;
    }
    Line::Line(Window &_w,int x,int y,int w,int h,Orientation orient):
        Line(_w,orient){
        
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;

        attr.user_rect = true;
    }
    Line::~Line(){}

    void Line::draw(Renderer &render){
        auto cliprect = render.get_cliprect();
        render.set_cliprect(rect);
        if(orientation == Orientation::H){
            //H
            int y = rect.y + rect.h / 2;
            //render.line();
            render.line(rect.x,y,rect.x + rect.w,y,{0,0,0,255});
        }
        else{
            //V
            int x = rect.x + rect.w / 2;
            render.line(x,rect.y,x,rect.y + rect.h,{0,0,0,255});
        }
        render.set_cliprect(cliprect);
    }
};