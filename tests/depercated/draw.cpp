#include <Btk/render.hpp>
#include <Btk/window.hpp>
#include <Btk/canvas.hpp>
#include <Btk/event.hpp>
#include <iostream>
#include "../resource/icon.xpm"
using namespace Btk;
int main(){
    Bounds bounds = {0,0,500,500};
    bounds.cast<Rect>();


    Window win("Drawing test",500,500);
    #if 1
    //Create a canvas
    auto &canvas = win.add<Canvas>(0,0,500,500);
    //load image
    auto  image = PixBuf::FromXPMArray(icon);
    FVec2 vec2 = {0,0};
    
    FVec2 pos = {0,0};//Image position
    FVec2 end_point = {500.0f,500.0f};
    float w = 0,h = 0;

    Gradient g(Gradient::YoungGrass);
    // g.add_color(0,Color{255,0,0});
    // g.add_color(1.0,Color{0,255,0});
    Brush brush(g);

    canvas.draw() = [&](Renderer &render){
        //render.box({0,0,500,500},{1,1,1});
        render.begin_path();
        render.rect(0,0,500,500);
        render.fill_color(20,40,45,255);
        render.fill();
        
        //render.draw_image(image,pos.x,pos.y,500,500);
        Rect cliprect = {100,0,200,200};
        FRect dst = {pos.x,pos.y,w,h};
        render.draw_image(image,&cliprect,&dst);
        if(vec2 != FVec2{0,0}){
            render.draw_line({0,0},vec2,Color{255,255,255,255});
        }
        render.begin_path();
        render.text_align(AlignVCenter | Align::HCenter);
        render.text_size(12);
        render.fill_color(255,255,255,255);
        render.text(vec2.x,vec2.y,"HelloWorld   PPP");
        render.fill();

        render.begin_path();
        render.stroke_color(208,208,208,255);
        render.rounded_rect(0,0,vec2.x,vec2.y,2);
        render.rounded_rect(50,50,50,50,5);
        render.stroke();
        //render.show_path_caches();

        // render.begin_path();
        // render.circle(vec2,100);
        // render.stroke();
        render.fill_circle(vec2,100,brush);

        render.begin_path();
        render.move_to(0,0);
        render.quad_to(vec2,end_point);
        render.move_to(0,0);
        render.bezier_to(end_point,vec2,{500.0f,500.0f});
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
        if(event.type() == Event::Mouse){
            end_point = static_cast<MouseEvent&>(event).position();
            canvas.redraw();
            return true;
        }
        if(event.type() == Event::KeyBoard){
            auto &ev = static_cast<KeyEvent&>(event);
            if(ev.keycode == Keycode::Kp_Plus){
                //+
                w += 100;
                h += 100;
                canvas.redraw();
            }
            else if(ev.keycode == Keycode::Kp_Minus){
                w -= 100;
                h -= 100;
                canvas.redraw();
            }
            // else if(ev.keycode == Keycode::){

            // }
            std::cout << w << " " << h << std::endl;
            return event.accept();
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
