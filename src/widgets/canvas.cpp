#include "../build.hpp"

#include <Btk/canvas.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>
namespace Btk{
    Canvas::Canvas() = default;
    Canvas::Canvas(int x,int y,int w,int h){
        rect = {x,y,w,h};
        attr.user_rect = true;
    }
    Canvas::~Canvas() = default;
    void Canvas::draw(Renderer &renderer){
        if(not draw_fn.empty()){
            auto viewport = renderer.get_viewport();
            renderer.set_viewport(rect);
            
            draw_fn(renderer);

            renderer.set_viewport(viewport);
        }
    }
    bool Canvas::handle(Event &event){
        if(event.type() == Event::SetContainer){
            event.accept();
            parent = event_cast<SetContainerEvent&>(event).container();
            return true;
        }
        if(event_fn.empty()){
            return false;
        }
        return event_fn(event);
    }
}