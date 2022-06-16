#include <Btk/signal/bind.hpp>
#include <Btk/scrollbar.hpp>
#include <Btk/container.hpp>
#include <Btk/imageview.hpp>
#include <Btk/layout.hpp>
#include <Btk/button.hpp>
#include <Btk/window.hpp>

int main(){
    Btk::Library lib;
    Btk::Window win("Test sliderable",500,500);
    auto &box = win.add<Btk::VBoxLayout>();

    auto &switch_lay = box.add<Btk::HBoxLayout>();
    auto &stacked = box.add<Btk::StackedWidget>();

    auto &btn1 = switch_lay.add<Btk::Button>("Switch to 0");
    auto &btn2 = switch_lay.add<Btk::Button>("Switch to 1");
    auto &btn3 = switch_lay.add<Btk::Button>("Switch to 2");

    stacked.add<Btk::SliderBar>(Btk::Horizontal).set_rect(
        0,100,500,50
    );
    stacked.add<Btk::Button>("I am a button on idx 1").set_rect(
        0,200,500,50
    );
    auto &view = stacked.add<Btk::ImageView>();
    view.set_rect(
        0,100,500,400
    );
    view.set_accept_drop();
    view.set_draw_boarder();
    
    auto callback = [&](int idx){
        stacked.set_current_widget(idx);
    };
    btn1.signal_clicked().connect(Btk::Bind(callback,0));
    btn2.signal_clicked().connect(Btk::Bind(callback,1));
    btn3.signal_clicked().connect(Btk::Bind(callback,2));

    box.set_stretch(switch_lay,0.1);

    win.mainloop();
}