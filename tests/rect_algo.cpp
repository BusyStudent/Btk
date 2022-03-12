#include <Btk/window.hpp>
#include <Btk/render.hpp>
#include <Btk/button.hpp>
#include <Btk/canvas.hpp>

#include <functional>
#include <random>

using namespace Btk;

struct App:public Window{
    App():Window("Rect algo test",500,600){
        add(btn1);
        add(btn2);
        add(canvas);

        btn1->set_rect(0,0,250,50);
        btn2->set_rect(250,0,250,50);

        btn1->set_text("Test CheckIntersectRectAndLine");
        btn2->set_text("Test RectCoverage");

        btn1->signal_clicked().connect(&App::test_intersect,this);
        btn2->signal_clicked().connect(&App::test_coverage,this);
        
        canvas->set_rect(0,50,500,550);
        canvas->draw() = std::bind(&App::draw,this,std::placeholders::_1);

        rand.seed(std::random_device()());
    }
    void draw(Renderer &render){
        const Rect canvas_rect = {0,50,500,550};
        render.draw_box(canvas_rect,{0,0,0});
        if(mode == None){
            return;
        }
        if(mode == Intersect){

            render.draw_rect(r1,{0,255,0});

            if(last_val){
                //Has Intersection
                render.draw_line(p1,p2,{0,255,255});
            }
            else{
                render.draw_line(p1,p2,{255,255,255});
            }
        }
        else if(mode == Coverage){
            render.draw_rect(r1,{0,255,0});
            render.draw_rect(r2,{0,255,0});
            render.draw_rect(out_r,{255,255,255});
        }
    }
    void test_intersect(){
        mode = Intersect;
        p1 = {rand_x(),rand_y()};
        p2 = {rand_x(),rand_y()};

        r1 = {
            rand_x(),
            rand_y(),
            rand_w(),
            rand_h(),
        };
        
        last_val = CheckIntersectRectAndLine(r1,p1,p2);

        Window::draw();
    }
    void test_coverage(){
        mode = Coverage;

        r1 = {
            rand_x(),
            rand_y(),
            rand_w(),
            rand_h(),
        };
        r2 = {
            rand_x(),
            rand_y(),
            rand_w(),
            rand_h(),
        };

        out_r = RectCoverage(r1,r2);

        Window::draw();
    }
    void reseed(){
        ticks ++;
        if(ticks == 50){
            ticks = 0;
            rand.seed(std::random_device()());
        }
    }
    int rand_x(){
        reseed();
        std::uniform_int_distribution<int> dis(0,500);
        return dis(rand);
    }
    int rand_y(){
        reseed();
        std::uniform_int_distribution<int> dis(50,600);
        return dis(rand);
    }
    int rand_w(){
        reseed();
        std::uniform_int_distribution<int> dis(10,100);
        return dis(rand);
    }
    int rand_h(){
        reseed();
        std::uniform_int_distribution<int> dis(10,100);
        return dis(rand);
    }

    Button *btn1 = new Button;
    Button *btn2 = new Button;
    Canvas *canvas = new Canvas;

    std::mt19937 rand;
    Rect r1;
    Rect r2;
    Rect out_r;
    Point p1;
    Point p2;

    bool last_val;
    Uint32 ticks = 0;

    enum{
        None,
        Intersect,
        Coverage
    } mode = None;
};

int main(){
    App app;
    app.mainloop();
}