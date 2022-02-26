#include <Btk/imageview.hpp>
#include <Btk/button.hpp>
#include <Btk/window.hpp>
#include <Btk/layout.hpp>
#include <Btk/label.hpp>

#include "resource/icon.xpm"

int main(){
    Btk::Window win("Layout Calc",500,500);
    win.set_resizeable();

    auto &lay = win.add<Btk::BoxLayout>(Btk::BoxLayout::LeftToRight);
    

    lay.add<Btk::Button>().set_text("Button 1");
    lay.add<Btk::Button>().set_text("Button 2");
    lay.add<Btk::Button>().set_text("Button 3");

    auto &sub = lay.add<Btk::BoxLayout>(Btk::BoxLayout::TopToBottom);

    auto &swit = sub.add<Btk::Button>();
    swit.set_text("Switch layout");
    swit.signal_clicked().connect([&](){
        lay.set_direction(Btk::BoxLayout::Direction((lay.direction() + 1)  % 4));
        sub.set_direction(Btk::BoxLayout::Direction((sub.direction() + 1)  % 4));
    });
    sub.add<Btk::ImageView>().set_image(Btk::PixBuf::FromXPMArray(icon));

    win.done();
    win.dump_tree();

    win.mainloop();
}