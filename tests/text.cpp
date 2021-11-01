#include <Btk/utils/textbuf.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/string.hpp>
#include <iostream>
int main(){
    #if 0
    Btk::TextBuffer buf;
    buf += "HelloWorld";
    buf += "FFFF";
    buf += "你好世界";
    std::cout << buf.to_string();
    buf.assign("你好");
    std::cout << buf.length() << std::endl;
    std::cout << buf.to_string();
    #endif
    Btk::Memdup(0);
    std::cout << Btk::ParseInt("  - 1") << std::endl;
    std::cout << Btk::ParseInt("1000") << std::endl;
    std::cout << Btk::ParseInt("1540") << std::endl;

    std::cout << Btk::ParseInt("1545450") << std::endl;
    std::cout << Btk::ParseHex("-0xFFFFFF") << std::endl;

    Btk::u8string str("你好 Helloケルベロス");
    std::cout << str[7] << std::endl;
    str[6] = u'我';
    str[7] = u'的';
    str[7] = u'的';
    std::cout << str << std::endl;
    std::cout << str.raw_length() << std::endl;
    
    for(auto ch:str){
        std::cout << ch  << ':' << ch.size() << std::endl;
    }

    str.prepend("Prefix前缀");
    std::cout << str << std::endl;

    Btk::u8string_view view("你好世界 sss sjajskaj");
    std::cout << view.length() << std::endl;
    std::cout << str.find("好") << std::endl;
    
    for(auto str:Btk::u8string_view("你好 哈哈哈 Hello ").split(" ")){
        std::cout << str << std::endl;
    }
    std::cout << Btk::u8string_view("你好").to_locale() << std::endl;
    std::cout << Btk::u8string_view("你好 哈哈哈 Hello ").substr(4) << std::endl;
    std::cout << Btk::u8string_view("你好 哈哈哈 Hello ").is_vaild() << std::endl;

    //Test string view iterator

    for(auto ch:Btk::u8string_view("你好 哈哈哈 Hello ")){
        std::cout << ch  << ':' << ch.size() << std::endl;
    }
    std::cout << str.end() - str.begin() << std::endl;
    std::cout << *(str.end() - 2) << std::endl;
    std::cout << str.length() << std::endl;

    std::cout << Btk::u8string_view("He你好ll").tolower() << std::endl;
    std::cout << Btk::u8string_view("He你好ll").toupper() << std::endl;
    std::cout << Btk::u8string_view("   He你好ll    ").trim() << std::endl;

}