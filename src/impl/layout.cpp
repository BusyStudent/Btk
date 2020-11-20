#include "../build.hpp"

#include <Btk/impl/render.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/layout.hpp>
#include <Btk/event.hpp>
#include <Btk/rect.hpp>
#include <list>
namespace Btk{
    void Layout::draw(Renderer &render){
        //draw layout widgets
        //Get ClipRect calcuate layout cliprect
        Rect cliprect = render.get_cliprect();
        Rect current;
        if(not cliprect.empty()){
            //on another container
            current.x = rect.x + cliprect.x;
            current.y = rect.y + cliprect.y;
            
            current.w = rect.w;
            current.h = rect.h;
        }
        else{
            current = rect;
        }
        render.set_cliprect(current);
        Btk_defer{
            //reset cliprect
            if(cliprect.empty()){
                render.set_cliprect(cliprect);
            }
        };
        for(auto w:widgets_list){
            if(visible() and (not w->rect.empty())){
                w->draw(render);
            }
        }
    }
    bool Layout::handle(Event &event){
        for(auto widget:widgets_list){
            if(widget->handle(event)){
                //Accept this event
                return true;
            }
        }
        return false;
    }
};