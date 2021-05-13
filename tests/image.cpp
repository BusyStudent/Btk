#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/Btk.hpp>
#include <iostream>
int main(){
    Btk::Window win("Test image",500,600);
    auto &view = win.add<Btk::ImageView>();
    win.on_dropfile([&view](Btk::u8string_view filename){
        view.set_image(
            Btk::PixBuf::FromFile(filename)
        );
    });
    win.on_resize([&win](int,int){
        //update window widgets postions
        win.update();
    });
    win.on_resize([](int w,int h){
        std::cout << "Window resized to " << w << ' ' << h << std::endl;
    });
    win.set_resizeable();
    
    win.done();
    win.mainloop();
}
