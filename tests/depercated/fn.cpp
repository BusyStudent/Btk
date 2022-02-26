#include <Btk/function.hpp>
#include <Btk/signal/bind.hpp>
#include <Btk/signal.hpp>
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
    void fn_with_args(int i){
        std::cout << "FN With Args : " << i << std::endl;
    }
    template<class T>
    void con(T &signal){
        connect(signal,&Test::fn);
    }

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
    //Test disconnect right now
    signal.connect(Btk::Bind(&Test::fn_with_args,&test,1)).disconnect();
    signal.emit();
    {
        //It should auto disconnect 
        Test t;
        t.con(signal);
        signal.connect(Btk::Bind(&Test::fn_with_args,&test,1));
        signal.connect(&Test::fn,&t);
        t.on_destroy([&t](){
            std::cout << "Object Test was destroyed" << std::endl; 
        });
        signal.dump_slots();
        t.dump_functors();
        signal.emit();
    }
    signal.emit();
}