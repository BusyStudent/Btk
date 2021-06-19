#include <Btk/Btk.hpp>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/msgbox/msgbox.hpp>
#include <Btk/utils/timer.hpp>
#include <Btk/container.hpp>
#include <Btk/imageview.hpp>
#include <Btk/textbox.hpp>
#include <Btk/button.hpp>
#include <Btk/event.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>
#include <Btk/label.hpp>
#include <iostream>
using Btk::Button;
using Btk::Label;
using Btk::Event;
using Btk::TextBox;
using Btk::ImageView;
using Btk::KeyEvent;
struct Hello:public Btk::Window{
    /**
     * @brief Construct a new Hello object
     * 
     */
    Hello();
    
    void onclose();
    void on_set_icon();
    void on_select(Btk::u8string_view fname);
    void on_defc_call();//Test defer_call
    void on_fullscreen();
    void on_dump_tree();//Test dump tree
    bool handle(Event &event);
    void show_text();
    void dump_tree();

    Button *close_btn;
    Button *seticon_btn;
    Button *show_btn;
    Button *defc_btn;
    Button *fsc_btn;// full screen
    Button *dmp_btn;
    TextBox *tbox;
    ImageView *img_view;
    
    bool fullscreen_flags = true;
};
Hello::Hello():Window("Hello",500,500){
    set_resizeable();

    close_btn = new Button("Close the window");
    seticon_btn = new Button("Set the icon");
    show_btn = new Button("Show the text");
    tbox = new TextBox();
    defc_btn = new Button("DeferCall");
    fsc_btn = new Button("FullScreen");
    dmp_btn = new Button("Dump Object Tree");
    
    img_view = new ImageView();

    auto group = new Btk::Group;

    group->add(close_btn);
    group->add(seticon_btn);
    group->add(show_btn);
    group->add(defc_btn);
    group->add(tbox);
    group->add(fsc_btn);
    group->add(img_view);
    group->add(dmp_btn);

    add(group);
    //Set the position
    close_btn->set_rect(0,0,100,40);
    seticon_btn->set_rect(100,100,300,40);
    tbox->set_rect(100,0,300,40);
    show_btn->set_rect(400,0,100,40);
    fsc_btn->set_rect(0,100,100,40);
    defc_btn->set_rect(400,100,100,40);
    img_view->set_rect(0,160,500,370);
    dmp_btn->set_rect(0,40,200,40);
    //Connect this signal
    //close_btn->signal_clicked().connect(&Hello::onclose,this);
    //Test HasSlots::connect
    connect(close_btn->signal_clicked(),&Hello::onclose);
    connect(dmp_btn->signal_clicked(),&Hello::dump_tree);

    seticon_btn->signal_clicked().connect(&Hello::on_set_icon,this);
    show_btn->signal_clicked().connect(&Hello::show_text,this);
    defc_btn->signal_clicked().connect(&Hello::on_defc_call,this);
    fsc_btn->signal_clicked().connect(&Hello::on_fullscreen,this);
    //Config
    img_view->set_draw_boarder();

    signal_event().connect(&Hello::handle,this);
}
void Hello::onclose(){
    Window::close();
}
void Hello::on_set_icon(){
    Btk::FSelectBox box;
    box.signal_async().connect(&Hello::on_select,this);
    box.show();
}
void Hello::on_select(Btk::u8string_view fname){
    if(fname.empty()){
        return;
    }
    set_icon(fname);
    img_view->set_image(fname);
}
void Hello::on_fullscreen(){
    set_fullscreen(fullscreen_flags);
    fullscreen_flags = not fullscreen_flags;
}
void Hello::show_text(){
    Btk::MessageBox msgbox("Show text",tbox->u8text());
    msgbox.show();
}
void Hello::on_defc_call(){
    defer_call(&Hello::show_text);
}
bool Hello::handle(Event &event){
    if(event.type() == Event::KeyBoard){
        auto &key = static_cast<KeyEvent&>(event);
        if(key.state == KeyEvent::Pressed){
            if(key.keycode == SDLK_F11){
                on_fullscreen();
            }
        }
    }
    return true;
}
void Hello::dump_tree(){
    Window::dump_tree();
}
int main(){
    //Btk::HideConsole();
    Hello app;
    app.dump_tree();
    app.mainloop();
}