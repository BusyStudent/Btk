#include <Btk/scrollbar.hpp>
#include <Btk/button.hpp>
#include <Btk/window.hpp>

int main(){
    Btk::Window win("Test sliderable",500,500);

    auto &slider = win.add<Btk::SliderBar>(Btk::Vertical);
    auto &s2 = win.add<Btk::SliderBar>(Btk::Horizontal);

    slider.set_value(60);
    slider.set_rect(0,0,30,500);

    s2.set_rect(0,0,500,30);

    win.mainloop();
}