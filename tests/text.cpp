#include <Btk/utils/textbuf.hpp>
#include <Btk/utils/mem.hpp>
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
}