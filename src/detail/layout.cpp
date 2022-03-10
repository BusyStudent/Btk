#include "../build.hpp"

#include <Btk/detail/scope.hpp>
#include <Btk/layout.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>
#include <Btk/rect.hpp>
#include <list>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdbool>


namespace Btk{
    //Init / Delete context
    Layout::Layout(){
        attr.layout = true;

    }
    Layout::~Layout(){

    }
    void Layout::set_rect(const Rect &r){
        Widget::set_rect(r);
        update();
    }
    bool Layout::handle(Event &event){
        if(Group::handle(event)){
            return true;
        }
        if(event.type() == Event::LayoutUpdate){
            event.accept();
            update();
            return true;
        }
        return false;
    }
}
namespace Btk{
    BoxLayout::BoxLayout(Direction d){
        set_direction(d);
    }
    BoxLayout::~BoxLayout(){

    }
    void BoxLayout::set_direction(Direction d){
        BTK_ASSERT(d >= LeftToRight and d <= BottomToTop);

        _direction = d;
        is_dirty = true;
        //Need redraw
        redraw();
    }
    void BoxLayout::update(){
        // if(not is_dirty){
        //     return;
        // }
        //Begin pack
        auto &list = get_childrens();

        auto update_horizontal = [this,list](){
            auto r = rectangle<float>();
            float w = r.w / list.size();
            float h = r.h;

            float x = r.x;
            float y = r.y;
            //Begin pack

            if(direction() == RightToLeft){
                for(auto iter = list.rbegin();iter != list.rend();++iter){
                    (*iter)->set_rectangle(x,y,w,h);

                    x += w;
                }
            }
            else{
                for(auto &child:list){
                    child->set_rectangle(x,y,w,h);
                    
                    x += w;
                }
            }
        };
        auto update_vertical = [this,list](){
            auto r = rectangle<float>();
            float w = r.w;
            float h = r.h / get_childrens().size();

            float x = r.x;
            float y = r.y;
            //Begin pack
            if(direction() == BottomToTop){
                for(auto iter = list.rbegin();iter != list.rend();++iter){
                    (*iter)->set_rectangle(x,y,w,h);
                    
                    y += h;
                }
            }
            else{
                //TopToBottom
                for(auto &child:childrens){
                    child->set_rectangle(x,y,w,h);
                    
                    y += h;
                }
            }
        };


        switch(direction()){
            case BottomToTop:
            case TopToBottom:{
                update_vertical();
                break;
            }
            case LeftToRight:
            case RightToLeft:{
                update_horizontal();
                break;
            }
        }

        is_dirty = false;
    }
    void BoxLayout::draw(Renderer &r){
        if(is_dirty){
            update();
        }
        Layout::draw(r);
    }

    HBoxLayout::HBoxLayout():BoxLayout(LeftToRight){

    }
    VBoxLayout::VBoxLayout():BoxLayout(TopToBottom){

    }

    HBoxLayout::~HBoxLayout() = default;
    VBoxLayout::~VBoxLayout() = default;


    GridLayout::GridLayout() = default;
    GridLayout::~GridLayout() = default;

    void GridLayout::update(){

    }
}