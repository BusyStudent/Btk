#include <Btk/layout.hpp>
#include <Btk/rect.hpp>
#include <list>
namespace Btk{
    void Layout::draw(Renderer &render){
        //draw layout widgets
        for(auto w:widgets_list){
            if(not(w->is_hided) and (not w->pos.empty())){
                w->draw(render);
            }
        }
    }
};