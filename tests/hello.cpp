#include <Btk/Btk.hpp>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/utils/timer.hpp>
#include <Btk/textbox.hpp>
#include <Btk/button.hpp>
#include <Btk/lable.hpp>
#include <iostream>
int main(){
    Btk::Timer timer;
    Btk::Window win("Hello",500,500);
    Btk::SetExceptionHandler([](std::exception *){
        return true;
    });
    //Btk::Window win2("Hello2",200,100);
    win.on_close([](){
        std::cout << "Window is on closed" << std::endl;
        return true;
    });
    win.on_dropfile([&win](std::string_view file){
        win.set_icon(file);
    });
    #if 1
    win.add<Btk::Lable>("Hello World").set_rect(
        0,0,100,100
    );
    #endif
    auto &button = win.add<Btk::Button>("Close the window");
    button.set_rect(
        200,200,100,30
    );
    button.sig_click().connect(
        &Btk::Window::close,
        &win
    );
    auto &btn2 = win.add<Btk::Button>(400,400,100,50);

    btn2.set_text("FSelectBox");
    btn2.sig_click().connect([&](){
        Btk::FSelectBox box("Select a file");
        box.sig_async().connect([&](std::string_view f){
            if(not f.empty()){
                win.set_icon(f);
            }
        });
        box.show();
    });

    win.add<Btk::TextBox>().set_rect(100,100,100,50);
    win.done();
    win.mainloop();
}