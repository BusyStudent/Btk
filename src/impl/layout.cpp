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
        for_each([](Widget *widget,Renderer &render){
            widget->draw(render);
        },render);
    }
    bool Layout::handle(Event &event){
        switch(event.type()){
            case Event::SetRect:{
                auto r = event_cast<SetRectEvent&>(event).rect();
                //Get rect
                rect = r;
                update();
                return event.accept();
            }
            case Event::SetContainer:{
                parent = event_cast<SetContainerEvent&>(event).container();
            }
            default:
                return Container::handle(event);
        }
    }
}
namespace Btk{
    GridLayout::GridLayout() = default;
    GridLayout::~GridLayout() = default;

    void GridLayout::update(){
        
    }
}