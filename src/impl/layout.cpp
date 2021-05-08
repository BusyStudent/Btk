#include "../build.hpp"

#include <Btk/impl/scope.hpp>
#include <Btk/layout.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>
#include <Btk/rect.hpp>
#include <list>
namespace Btk{
    
    void Layout::draw(Renderer &render){
        //draw layout widgets
        //Get ClipRect calcuate layout cliprect
        for(auto ch:childrens){
            ch->draw(render);
        }
    }
}
namespace Btk{
    #if 0
    GridLayout::GridLayout() = default;
    GridLayout::~GridLayout() = default;

    void GridLayout::update(){
        
    }
    #endif
}