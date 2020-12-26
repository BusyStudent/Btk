#include <Btk/utils/textbuf.hpp>
#include <iostream>
int main(){
    Btk::TextBuffer buf;
    buf += "HelloWorld";
    buf += "FFFF";
    buf += "你好世界";
    std::cout << buf.to_string();
    buf.assign("你好");
    std::cout << buf.length() << std::endl;
    std::cout << buf.to_string();
    int num = 0;
    
    Btk::Memdup(&num);
}