#include "../build.hpp"

#include <Btk.hpp>
#include <Btk.h>

#define BTK_CAPI_BEGIN extern "C"{
#define BTK_CAPI_END }
//Enable/Disable TypeChecking
#ifdef BTK_UNSAFE
    #define BTK_TYPE_CHK(WIDGET,TYPE) 
    #define BTK_NUL_CHK(VAL)
#else
    #define BTK_NUL_CHK(VAL) \
        if(VAL == nullptr){\
            Btk_SetError("nullptr");\
            return;\
        }

    #define BTK_TYPE_CHK(WIDGET,TYPE) \
        if(!Btk_Is##TYPE(BTK_WIDGET(WIDGET)))\
        {   Btk_SetError("TypeError:(%s,%s)",#TYPE,Btk::get_typename(WIDGET).c_str());\
            return;}
#endif

static thread_local std::string global_error;
BTK_CAPI_BEGIN
//Global Init/Quit
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
//Widget

void Btk_UpdateRect(BtkWidget *widget,int x,int y,int w,int h){
    BTK_NUL_CHK(widget);
    widget->set_rect(x,y,w,h);
}
void Btk_AtDelete(BtkWidget *widget,Btk_callback_t fn,void*param){
    BTK_NUL_CHK(widget);
    widget->on_destroy(fn,param);
}
void Btk_Delete(BtkWidget *widget){
    delete widget;
}
//Button
void Btk_AtButtonClicked(BtkButton *button,Btk_callback_t fn,void *param){
    BTK_NUL_CHK(button);
    BTK_NUL_CHK(fn);
    BTK_TYPE_CHK(button,Button);

    button->sig_click().connect([&fn,param](){
        fn(param);
    });
}
//Window
BtkWindow *Btk_NewWindow(const char *title,int w,int h){
    return new Btk::Window(title,w,h);
}
//Error
const char *Btk_GetError(){
    return global_error.c_str();
}

void Btk_SetError(const char *fmt,...){
    int strsize;

    //Get the size of the string
    va_list varg;
    va_start(varg,fmt);
    #ifdef _WIN32
    strsize = _vscprintf(fmt,varg);
    #else
    strsize = vsnprintf(nullptr,0,fmt,varg);
    #endif
    va_end(varg);
    
    global_error.clear();
    global_error.resize(strsize);

    //start formatting
    va_start(varg,fmt);
    vsprintf(&global_error[0],fmt,varg);
    va_end(varg);
}

BTK_CAPI_END