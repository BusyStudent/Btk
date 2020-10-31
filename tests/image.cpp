#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/Btk.hpp>
int main(){
    Btk::Window win("Test image",500,600);
    auto &view = win.add<Btk::ImageView>();
    win.on_dropfile([&view](std::string_view filename){
        view.set_image(
            Btk::PixBuf::FromFile(filename)
        );
    });
    win.on_resize([&win](int,int){
        //update window widgets postions
        win.update();
    });
    win.update();
    win.set_resizeable();
    win.mainloop();
}
