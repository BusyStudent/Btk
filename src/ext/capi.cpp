#include "../build.hpp"

#include <Btk.hpp>
#include <Btk.h>

extern "C"{

bool Btk_Init(){
    try{
        Btk::Init();
        return true;
    }
    catch(...){
        return false;
    }
}
int  Btk_Run(){
    return Btk::run();
}
void Btk_AtDelete(BtkWidget *widget,void(*fn)(void*),void*param){
    widget->on_destroy(fn,param);
}
void Btk_Delete(BtkWidget *widget){
    delete widget;
}

BtkWindow *Btk_NewWindow(const char *title,int w,int h){
    return new Btk::Window(title,w,h);
}

}