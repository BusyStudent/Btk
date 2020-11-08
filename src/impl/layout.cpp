#include <Btk/impl/render.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/layout.hpp>
#include <Btk/rect.hpp>
#include <list>
namespace Btk{
    void Layout::draw(Renderer &render){
        //draw layout widgets
        //Get ViewPort calcuate layout viewport
        Rect viewport = render.get_viewport();
        Rect current;
        if(not viewport.empty()){
            //on another container
            current.x = pos.x + viewport.x;
            current.y = pos.y + viewport.y;
            
            current.w = pos.w;
            current.h = pos.h;
        }
        else{
            current = pos;
        }
        render.set_viewport(current);
        Btk_defer{
            //reset viewport
            if(viewport.empty()){
                render.set_viewport(viewport);
            }
        };
        for(auto w:widgets_list){
            if(visible() and (not w->pos.empty())){
                w->draw(render);
            }
        }
    }
};