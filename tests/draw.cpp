#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include "icon.xpm"
using namespace Btk;
int main(){
    Window win("Drawing test",500,500);
    //Create a canvas
    auto &canvas = win.add<Canvas>(0,0,500,500);
    //load image
    auto  image = PixBuf::FromXPMArray(icon);

    canvas.draw() = [&](Renderer &render){
        render.box({0,0,500,500},{1,1,1});
        render.copy(image,nullptr,{0,0,500,500});
    };
    win.on_dropfile([&](std::string_view fname){
        image = PixBuf::FromFile(fname);
    });
    win.mainloop();
}
