#include <Btk/signal/function.hpp>
#include <Btk/signal/signal.hpp>
#include <Btk/module.hpp>
#include <iostream>
#include <functional>
void func(int i){
    std::cout << "Hello World :"  << i << std::endl;
};
struct Test:public Btk::HasSlots{
    void fn(){
        std::cout << "Hello World from Class Test" << std::endl;
    };
};
int main(){
    int i = 0;
    Btk::Function<void(int)> fn([](int i) -> void{
        std::cout << "Hello from lambda :" << i << std::endl;
    });
    fn(i);
    auto fn2 = fn;
    
    fn2(10);
    
    Btk::Function<void(int)> n(func);
    n(10);
    //auto f = std::bind(func,1);
    std::cout << i << std::endl;
    
    Test test;
    Btk::Signal<void()> signal;
    
    signal.connect([&](){
        i = 10;
        std::cout << "Hello World from signal slot :"  << i << std::endl;
    });
    signal.connect(&Test::fn,&test);
    signal.emit();
}