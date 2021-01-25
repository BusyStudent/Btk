#include <Btk/scrollbar.hpp>
#include <Btk.hpp>
int main(){
    Btk::Init();
    Btk::Window win("ScrollBar test",600,600);
    auto &bar = win.add<Btk::ScrollBar>(Btk::Orientation::H);
    win.mainloop();
}