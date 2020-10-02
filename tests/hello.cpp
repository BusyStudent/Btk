#include <Btk/Btk.hpp>
#include <iostream>
int main(){
    Btk::Window win("Hello",100,100);
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
    win.mainloop();
}