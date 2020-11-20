#include <Btk/async/async.hpp>
#include <iostream>
int main(){
    using namespace Btk;
    Async([](){
        std::cout << "Hello Wolrd" << std::endl;
    }).rt_lunch();
}