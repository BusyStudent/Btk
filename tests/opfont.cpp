#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
int main(){
    //this file dosnet exists
    Btk::Init();
    try{
        Btk::Font::FromFile("xxxx",45);
    }
    catch(...){

    }
}