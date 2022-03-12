#include <Btk/async.hpp>
#include <Btk/Btk.hpp>
#include <iostream>
#include <thread>
int main(){
    using namespace Btk;
    Init();
    Async([]() -> std::string{
        std::cout << "Hello Wolrd" << std::endl;
        return std::string("Hello World");
    })->connect([](std::string_view view){
        std::cout << view << std::endl;
        Btk::Exit();
    });
    Btk::run();
}