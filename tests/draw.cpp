#include <Btk/msgbox/msgbox.hpp>
#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include <Btk/event.hpp>
#include <iostream>
#include "icon.xpm"
using namespace Btk;
int main(){
    Window win("Drawing test",500,500);
    #if 1
    //Create a canvas
    auto &canvas = win.add<Canvas>(0,0,500,500);
    //load image
    auto  image = PixBuf::FromXPMArray(icon);
    Vec2 vec2 = {0,0};
    
    Vec2 pos = {0,0};//Image position

    canvas.draw() = [&](Renderer &render){
        //render.box({0,0,500,500},{1,1,1});
        render.begin_path();
        render.rect(0,0,500,500);
        render.fill_color(0,0,0,255);
        render.fill();
        
        //render.draw_image(image,pos.x,pos.y,500,500);
        FRect cliprect = {100,0,200,200};
        FRect dst = {100,100,200,200};
        render.draw_image(image,&cliprect,&dst);
        if(vec2 != Vec2{0,0}){
            render.line({0,0},vec2,{0,0,0,255});
        }
        render.begin_path();
        render.text_align(Align::Center,Align::Center);
        render.text_size(12);
        render.text(vec2.x,vec2.y,"HelloWorld   PPP");
        render.fill_color(255,255,255,255);
        render.fill();

        render.begin_path();
        render.stroke_color(208,208,208,255);
        render.rounded_rect(0,0,vec2.x,vec2.y,2);
        render.rounded_rect(50,50,50,50,5);
        render.stroke();
        //render.show_path_caches();

        render.begin_path();
        render.circle(vec2,100);
        render.stroke();
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
    win.on_dropfile([&](u8string_view fname){
        image = PixBuf::FromFile(fname);
    });
    #endif
    #if 0
    struct Test:public Widget{
        Test(){

        }
        Texture texture;
        void draw(Renderer &render){
            auto  image = PixBuf::FromXPMArray(icon);
            if(texture.empty()){
                texture = render.create(500,500);
                render.set_target(texture);
                //The texture is uninited
                //Clear it
                render.clear(0,0,0,0);

                render.begin_path();
                render.move_to(0,0);
                render.line_to(500,500);
                render.move_to(0,500);
                render.line_to(500,0);
                render.close_path();
                render.stroke();
                //render.rounded_rect({0,0,200,200},2,{0,0,0,255});
                render.begin_path();
                render.rect(50,50,30,30);
                render.fill_color(50,20,40,55);
                render.fill();
                render.reset_target();

                //Test clone
                //texture = texture.clone();
                texture.dump().save_bmp("A.bmp");
            }
            
            
            render.draw_image(texture,nullptr,nullptr);
        }
    };
    win.add<Test>();
    #endif
    win.mainloop();
}
