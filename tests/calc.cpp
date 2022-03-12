#include <Btk/imageview.hpp>
#include <Btk/scrollbar.hpp>
#include <Btk/button.hpp>
#include <Btk/window.hpp>
#include <Btk/layout.hpp>
#include <Btk/label.hpp>

#include <cstdlib>
#include "resource/icon.xpm"

int main(){
    Btk::Window win("Layout Calc",500,500);
    win.set_resizeable();

    auto &global = win.add<Btk::BoxLayout>(Btk::BoxLayout::TopToBottom);
    //Add silder to test the layout
    auto &silder = global.add<Btk::SliderBar>(Btk::Horizontal);
    global.set_stretch(silder,0.1f);
    auto &lay = global.add<Btk::BoxLayout>(Btk::BoxLayout::LeftToRight);
    
    lay.add_stretch(1.0f);
    lay.add<Btk::Button>().set_text("Button 1");
    lay.add<Btk::Button>().set_text("Button 2");
    lay.add<Btk::Button>().set_text("Button 3");
    lay.set_spacing(10);

    lay.set_content_margins(0,20,0,20);

    auto &sub = lay.add<Btk::BoxLayout>(Btk::BoxLayout::TopToBottom);

    auto &swit = sub.add<Btk::Button>();
    swit.set_text("Switch layout");
    swit.signal_clicked().connect([&](){
        lay.set_direction(Btk::BoxLayout::Direction((lay.direction() + 1)  % 4));
        sub.set_direction(Btk::BoxLayout::Direction((sub.direction() + 1)  % 4));
    });
    auto &view = sub.add<Btk::ImageView>();
    view.set_image(Btk::PixBuf::FromXPMArray(icon));
    view.set_accept_drop();

    sub.set_spacing(10);
    //Use silder to set spacing
    silder.signal_value_changed().connect([&](int v){
        lay.set_spacing(v);
        sub.set_spacing(v);
    });

    win.done();
    win.dump_tree();

    win.mainloop();    
}