#include <Btk/utils/mem.hpp>
#include <Btk/string.hpp>
#include <Btk/regex.hpp>
#include <iostream>


using namespace Btk;

void test_str_replace(){
    u8string en_str = u8"Hello World";
    u8string lt_str = u8"ສະ​ບາຍ​ດີ​ຊາວ​ໂລກ";
    u8string ru_str = u8"Привет мир";
    u8string jp_str = u8"こんにちは世界";
    u8string cn_str = u8"你好世界";

    // ru_str.insert(8,"(World)");
    // jp_str.insert(4,"(World)");

    // std::cout << ru_str << std::endl;
    // std::cout << jp_str << std::endl;

    //Replace ru world to en
    ru_str.replace(7,3,en_str);

    std::cout << ru_str << std::endl;
    std::cout << jp_str << std::endl;
}
void test_regex(){
    Btk::Regex regex("[0-9]+");
    for(auto &each:regex.match("abc123def1111aaa555")){
        std::cout << "Regex out:" <<each << std::endl;
    }
    std::cout << regex.replace_to("abc123def1111aaa555","Number") << std::endl;
}

int main(){
    test_str_replace();
    test_regex();
    return 0;
}