#include <Btk/async/async.hpp>
#include <Btk/Btk.hpp>
#include <iostream>
#include <thread>
int main(){
    using namespace Btk;
    Init();
    Async([](){
        std::cout << "Hello Wolrd" << std::endl;
    });
}