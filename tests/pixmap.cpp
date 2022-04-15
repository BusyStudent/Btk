#include <Btk/pixels.hpp>

#include "resource/icon.xpm"

int main(){
    auto buf = Btk::PixBuf::FromXPMArray(icon);
    auto resized = buf.resize(1000,1000);

    Btk::Brush b1(buf);
    Btk::Brush b2;

    b2 = b1;
    b1 = Btk::Color(255,0,0,255);
    b1 = b2;
    b2 = Btk::Color(0,255,0,255);

    b2 = buf;

    // resized.save("hello.png");
}