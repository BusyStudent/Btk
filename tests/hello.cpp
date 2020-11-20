#include <Btk/Btk.hpp>
#include <Btk/button.hpp>
#include <Btk/lable.hpp>
#include <iostream>
int main(){
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
    win.add<Btk::Button>(400,400,100,50).set_text("Button");;
    win.done();
    win.mainloop();
}