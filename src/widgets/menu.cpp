#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/exception.hpp>
#include <Btk/menu.hpp>

namespace Btk{
    //TODO
    void Menu::set_parent(Widget *parent){
        Widget::set_parent(parent);
        if(window() != nullptr){
            if(window()->menu_bar != nullptr){
                //Already has a menu bar
                throwRuntimeError("Already has a menu");
            }
            //Set the menu bar
            window()->menu_bar = this;
        }
    }

    Menu::Menu() = default;
    Menu::~Menu() = default;
}