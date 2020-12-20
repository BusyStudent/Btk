#include <Btk/utils/timer.hpp>
#include <iostream>
#include <thread>
using Btk::Timer;
int main(){
    Timer timer;
    timer.set_interval(100);
    timer.set_callback([&timer](){
        std::cout << "Timeout interval:" << timer.interval() << std::endl;
    }).start();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.set_interval(10);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    timer.stop();
}