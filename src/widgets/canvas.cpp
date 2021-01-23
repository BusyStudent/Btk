#include "../build.hpp"

#include <Btk/canvas.hpp>
#include <Btk/render.hpp>
namespace Btk{
    Canvas::Canvas(Container &parent):Widget(parent){

    }
    Canvas::Canvas(Container &parent,int x,int y,int w,int h):
        Canvas(parent){
            
        rect = {x,y,w,h};
        attr.user_rect = true;
    }
    Canvas::~Canvas(){}
    void Canvas::draw(Renderer &renderer){
        if(not draw_fn.empty()){
            auto viewport = renderer.get_viewport();
            renderer.set_viewport(rect);
            
            draw_fn(renderer);

            renderer.set_viewport(viewport);
        }
    }
    bool Canvas::handle(Event &event){
        if(event_fn.empty()){
            return false;
        }
        return event_fn(event);
    }
}