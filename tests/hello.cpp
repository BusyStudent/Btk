#include <Btk/Btk.hpp>
#include <Btk/msgbox/fselect.hpp>
#include <Btk/msgbox/msgbox.hpp>
#include <Btk/utils/timer.hpp>
#include <Btk/textbox.hpp>
#include <Btk/button.hpp>
#include <Btk/themes.hpp>
#include <Btk/event.hpp>
#include <Btk/label.hpp>
#include <iostream>
struct Hello:public Btk::Window{
    using Button = Btk::Button;
    using Label =  Btk::Label;
    using TextBox = Btk::TextBox;
    /**
     * @brief Construct a new Hello object
     * 
     */
    Hello();
    
    void onclose();
    void on_set_icon();
    void on_select(std::string_view fname);
    void show_text();

    Button *close_btn;
    Button *seticon_btn;
    Button *show_btn;
    TextBox *tbox;
};
Hello::Hello():Window("Hello",500,500){
    set_resizeable();

    close_btn = new Button("Close the window");
    seticon_btn = new Button("Set the icon");
    show_btn = new Button("Show the text");
    tbox = new TextBox();

    add(close_btn);
    add(seticon_btn);
    add(show_btn);
    add(tbox);
    //Set the position
    close_btn->set_rect(0,0,100,40);
    seticon_btn->set_rect(100,100,300,40);
    tbox->set_rect(100,0,300,40);
    show_btn->set_rect(400,0,100,40);
    //Connect this signal
    close_btn->signal_clicked().connect(&Hello::onclose,this);
    seticon_btn->signal_clicked().connect(&Hello::on_set_icon,this);
    show_btn->signal_clicked().connect(&Hello::show_text,this);
}
void Hello::onclose(){
    Window::close();
}
void Hello::on_set_icon(){
    Btk::FSelectBox box;
    box.signal_async().connect(&Hello::on_select,this);
    box.show();
}
void Hello::on_select(std::string_view fname){
    set_icon(fname);
}
void Hello::show_text(){
    Btk::MessageBox msgbox("Show text",tbox->u8text());
    msgbox.show();
}
int main(){
    Hello app;
    app.mainloop();
}