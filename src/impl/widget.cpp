//Widget impl
#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/widget.hpp>
namespace Btk{
    Widget::~Widget(){
        
    }
    Container::Container(){

    }
    //delete container
    Container::~Container(){
        for(auto iter = widgets_list.begin();iter != widgets_list.end();){
            delete *iter;
            iter = widgets_list.erase(iter);
        }
    }
};