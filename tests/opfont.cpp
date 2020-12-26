#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <iostream>
int main(){
    std::cout << Btk::FontUtils::GetFileByName("Arial") << std::endl;
    auto fontset = Btk::FontUtils::GetFontList();
    for(auto font:fontset){
        std::cout << font.file() << std::endl;
    }
}