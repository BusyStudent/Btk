#include "../build.hpp"

#include <Btk.hpp>
#include <Btk.h>

#define BTK_NUL_CHK(VAL) if(VAL == nullptr){return;}
#define BTK_CAPI_BEGIN extern "C"{
#define BTK_CAPI_END }
static thread_local std::string global_error;
BTK_CAPI_BEGIN

bool Btk_Init(){
    try{
        Btk::Init();
        return true;
    }
    catch(std::exception &exp){
        global_error = exp.what();
        return false;
    }
    catch(...){
        global_error.clear();
        return false;
    }
}
int  Btk_Run(){
    return Btk::run();
}
void Btk_UpdateRect(BtkWidget *widget,int x,int y,int w,int h){
    BTK_NUL_CHK(widget);
    widget->set_rect(x,y,w,h);
}
void Btk_AtDelete(BtkWidget *widget,void(*fn)(void*),void*param){
    BTK_NUL_CHK(widget);
    widget->on_destroy(fn,param);
}
void Btk_Delete(BtkWidget *widget){
    delete widget;
}

BtkWindow *Btk_NewWindow(const char *title,int w,int h){
    return new Btk::Window(title,w,h);
}
const char *Btk_GetError(){
    return global_error.c_str();
}


BTK_CAPI_END