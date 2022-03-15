#include <Btk/pixels.hpp>
#include "Btk/Btk.hpp"

#include "resource/icon.xpm"

int main(){
    Btk::Init();
    auto buf = Btk::PixBuf::FromXPMArray(icon);
    auto resized = buf.resize(1000,1000);
    auto blur1 = buf.blur(10);
    resized.save("hello.bmp");
    blur1.save("hello_test.bmp");
}