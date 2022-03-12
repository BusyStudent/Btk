#include <Btk/pixels.hpp>

#include "resource/icon.xpm"

int main(){
    auto buf = Btk::PixBuf::FromXPMArray(icon);
    auto resized = buf.resize(1000,1000);
    
    resized.save("hello.png");
}