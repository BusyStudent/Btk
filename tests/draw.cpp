#include <Btk/msgbox/msgbox.hpp>
#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include <Btk/event.hpp>
#include <iostream>
#include "icon.xpm"
using namespace Btk;
int main(){
    Window win("Drawing test",500,500);
    //Create a canvas
    auto &canvas = win.add<Canvas>(0,0,500,500);
    //load image
    auto  image = PixBuf::FromXPMArray(icon);
    Vec2 vec2 = {0,0};
    
    Vec2 pos = {0,0};//Image position

    canvas.draw() = [&](Renderer &render){
        render.box({0,0,500,500},{1,1,1});
        render.copy(image,nullptr,{pos.x,pos.y,500,500});
        if(vec2 != Vec2{0,0}){
            render.line({0,0},vec2,{0,0,0,255});
        }
    };
    canvas.handle() = [&](Event &event){
        if(event.type() == Event::Wheel){
            event.accept();
            auto &ev = static_cast<WheelEvent&>(event);
            std::cout << "Wheel x " << ev.x << " y " << ev.y << std::endl;
            if(ev.y > 0){
                pos.y += image->h / 100;
            }
            else{
                pos.y -= image->h / 100;
            }
            canvas.redraw();
            return true;
        }
        if(event.type() == Event::Motion){
            event.accept();
            auto &ev = static_cast<MotionEvent&>(event);
            vec2 = ev.position();
            canvas.redraw();
            return true;
        }
        if(event.type() == Event::Click){
            event.accept();
            MessageBox box("HelloWorld","FFF",MessageBox::Warn);
            box.show();
            return true;
        }
        return false;
    };
    win.on_dropfile([&](std::string_view fname){
        image = PixBuf::FromFile(fname);
    });
    win.mainloop();
}
