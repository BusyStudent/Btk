#include "../build.hpp"

#include <Btk/detail/scope.hpp>
#include <Btk/exception.hpp>
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
        #ifndef NDEBUG
        //For all children and print its userdata ptr
        for(auto &c:childrens){
            BTK_LOGINFO("Widget '%s' %p item : %p",BTK_typenameof(c),c,c->userdata(BTK_LAYOUT_ITEMPTR));
        }
        #endif
        clear_items();
    }
    void Layout::set_rect(const Rect &r){
        Widget::set_rect(r);
        invalidate();
    }
    bool Layout::handle(Event &event){
        if(Group::handle(event)){
            return true;
        }
        if(event.type() == Event::LayoutUpdate){
            event.accept();

            if(is_dirty){
                update();
                is_dirty = false;
            }
            return true;
        }
        return false;
    }
    //Add item and 
    bool Layout::add(Widget *w){
        if(not Group::add(w)){
            return false;
        }
        Item *item = alloc_item(w);
        if(item != nullptr){
            items.push_back(item);
            //Associate the widget with the item
            w->set_userdata(BTK_LAYOUT_ITEMPTR,item);
            invalidate();
            return true;
        }
        //Should we remove the widget from the group?
        return false;
    }
    bool Layout::detach(Widget *w){
        if(not Group::detach(w)){
            return false;
        }
        for(auto it = items.begin(); it != items.end(); ++it){
            if((*it)->widget == w){
                //Unaasociate the widget with the item
                w->set_userdata(BTK_LAYOUT_ITEMPTR,nullptr);
                delete *it;
                items.erase(it);
                invalidate();
                return true;
            }
        }
        //It should not happen
        //Abort program
        BTK_LOGWARN("Layout::detach: Widget not found");
        abort();
    }
    void Layout::lower_widget(Widget *w){
        if(w == nullptr){
            throwRuntimeError("unknown widget in container");
        }
        Group::lower_widget(w);
        //Move the item to the end of the list
        for(auto it = items.begin(); it != items.end(); ++it){
            if((*it)->widget == w){
                items.splice(items.end(),items,it);
                invalidate();
                return;
            }
        }
        invalidate();
    }
    void Layout::raise_widget(Widget *w){
        if(w == nullptr){
            throwRuntimeError("unknown widget in container");
        }

        Group::raise_widget(w);
        //Move the item to the begining of the list
        for(auto it = items.begin(); it != items.end(); ++it){
            if((*it)->widget == w){
                items.splice(items.begin(),items,it);
                invalidate();
                return;
            }
        }
        invalidate();
    }
    void Layout::clear(){
        Group::clear();
        clear_items();
    }
    void Layout::clear_items(){
        for(auto it = items.begin(); it != items.end();){
            delete *it;
            it = items.erase(it);            
        }
    }
    auto Layout::index_item(int idx) -> Item*{
        if(idx < 0 or idx >= items.size()){
            return nullptr;
        }
        auto it = items.begin();
        std::advance(it, idx);
        return *it;
    }
    void Layout::draw(Renderer &r){
        if(is_dirty){
            update();
            is_dirty = false;
        }
        Group::draw(r);
    }
    void Layout::invalidate(){
        is_dirty = true;
        redraw();
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

        invalidate();
    }
    void BoxLayout::set_spacing(float spacing){
        _spacing = spacing;
        invalidate();
    }
    void BoxLayout::add_stretch(float stretch){
        Item *item = new Item;
        item->stretch = stretch;
        items.push_back(
            item
        );
        invalidate();
    }
    void BoxLayout::set_stretch(Widget *w,float stretch){
        auto it = index_item(w);
        if(it == nullptr){
            throwRuntimeError("unknown widget in container");
        }
        it->stretch = stretch;
    }

    auto BoxLayout::alloc_item(Widget *w) -> Item*{
        Item *item = new Item;
        item->widget = w;
        return item;
    }
    void BoxLayout::update(){
        // if(not is_dirty){
        //     return;
        // }
        //Begin pack
        auto rect  = margin.apply(rectangle<float>());
        auto &list = get_childrens();
        //Calc all item factors
        float factors = 0;
        for(auto item:items){
            factors += item->stretch;
        }
        //In vertical direction, we need to know the height of the layout
        if(_direction == LeftToRight or _direction == RightToLeft){
            //V
            float x = rect.x;
            float y = rect.y;
            float h = rect.h;
            float w = rect.w;
            //Calc the useable width after the spacing
            w -= _spacing * (items.size() - 1);

            if(_direction == RightToLeft){
                for(auto iter = items.rbegin(); iter != items.rend(); ++iter){
                    auto item = *iter;
                    item->x = x;
                    item->y = y;
                    item->h = h;

                    item->w = w / factors * item->stretch;

                    //Step by spacing
                    x += item->w + _spacing;
                }
            }
            else{
                for(auto item:items){
                    item->x = x;
                    item->y = y;
                    item->h = h;

                    item->w = w / factors * item->stretch;

                    //Step by spacing
                    x += item->w + _spacing;
                }
            }
        }
        else{
            //H
            float x = rect.x;
            float y = rect.y;
            float w = rect.w;
            float h = rect.h;

            //Calc useable height after the spacing
            h -= _spacing * (items.size() - 1);

            if(_direction == BottomToTop){
                for(auto iter = items.rbegin(); iter != items.rend(); ++iter){
                    auto item = *iter;
                    item->x = x;
                    item->y = y;
                    item->w = w;

                    item->h = h / factors * item->stretch;

                    //Step by spacing
                    y += item->h + _spacing;
                }
            }
            else{
                for(auto item:items){
                    item->x = x;
                    item->y = y;
                    item->w = w;

                    item->h = h / factors * item->stretch;

                    //Step
                    y += item->h + _spacing;
                }
            }
        }

        //End pack
        for(auto item:items){
            if(item->widget == nullptr){
                continue;
            }
            item->widget->set_rectangle(
                std::round(item->x),
                std::round(item->y),
                std::round(item->w),
                std::round(item->h)
            );
        }
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
        BTK_FIXME("It has not been implemented yet");
    }
}