#include <Btk/window.hpp>
#include <Btk/dialog.hpp>
#include <Btk/menu.hpp>
#include <Btk/Btk.hpp>
#include <iostream>

using namespace Btk;

int main(){
    Window win("HelloWorld",100,500);
    win.set_resizeable();

    auto &bar = win.add<MenuBar>();
    auto *file = bar.add_menu("File");

    auto act = new Action(
        "Open"
    );

    auto close = new Action(
        "Close"
    );

    file->add_action(act);
    file->add_action(close);

    close->signal_triggered().connect([](){
        Exit();
    });
    act->signal_triggered().connect([&](){
        std::cout << "File clicked" << std::endl;
        FileDialog box("Select your file");
        box.set_parent(win);
        box.run();

    });

    bar.add_action(
        new Action("Edit")
    );

    auto *sub = bar.add_menu("SubMenu");
    sub->add_action(
        new Action("SubMenuItem")
    );
    auto sub2 = sub->add_menu("SubMenu2");
    sub2->add_action(
        new Action("SubMenu2Item")
    );

    win.mainloop();
}