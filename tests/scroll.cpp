#include <Btk/progressbar.hpp>
#include <Btk/scrollbar.hpp>
#include <Btk.hpp>
int main(){
    Btk::Init();
    Btk::Window win("ScrollBar test",600,600);
    auto &bar = win.add<Btk::ScrollBar>(Btk::Orientation::V);
    auto &bar1 = win.add<Btk::ScrollBar>(Btk::Orientation::H);
    // bar.set_rect({0,0,600,10});
    auto &b = win.add<Btk::ProgressBar>();
    b.set_rect(0,0,200,20);

    bar.signal_moved().connect(&Btk::ProgressBar::set_value,&b);

    win.mainloop();
}