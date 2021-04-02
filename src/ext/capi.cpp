#include "../build.hpp"


#define BTK_CAPI_BEGIN extern "C"{
#define BTK_CAPI_END }
//Enable/Disable TypeChecking
#ifdef BTK_UNSAFE
    #define BTK_TYPE_CHK(WIDGET,TYPE) 
    #define BTK_TYPE_CHK2(WIDGET,TYPE,ERR_RET)

    #define BTK_NUL_CHK(VAL)
    #define BTK_NUL_CHK2(VAL,RET)
    /**
     * @brief Begin the catch
     * 
     */
    #define BTK_BEGIN_CATCH()
    /**
     * @brief End the catch
     * 
     */
    #define BTK_END_CATCH()
    /**
     * @brief End the catch
     * @param ERR_RET The return value if the exception happended
     */
    #define BTK_END_CATCH2(ERR_RET)


#else
    #define BTK_NUL_CHK(VAL) \
        if(VAL == nullptr){\
            Btk_SetError("nullptr");\
            return;\
        }
    #define BTK_NUL_CHK2(VAL,RET) \
        if(VAL == nullptr){\
            Btk_SetError("nullptr");\
            return RET;\
        }

    #define BTK_TYPE_CHK(WIDGET,TYPE) \
        if(!Btk_Is##TYPE(BTK_WIDGET(WIDGET)))\
        {   set_typeerror(#TYPE,WIDGET);\
            return;}
    #define BTK_TYPE_CHK2(WIDGET,TYPE,ERR_RET) \
        if(!Btk_Is##TYPE(BTK_WIDGET(WIDGET)))\
        {   set_typeerror(#TYPE,WIDGET);\
            return ERR_RET;}

    /**
     * @brief Begin the catch
     * 
     */
    #define BTK_BEGIN_CATCH() try{
    /**
     * @brief End the catch
     * 
     */
    #define BTK_END_CATCH() } \
        catch(std::exception &exp){\
            Btk_SetError("%s",exp.what());\
        }\
        catch(...){\
            Btk_SetError("Unknown Exception");\
        }
    /**
     * @brief End the catch
     * @param ERR_RET The return value if the exception happended
     */
    #define BTK_END_CATCH2(ERR_RET) } \
        catch(std::exception &exp){\
            Btk_SetError("%s",exp.what());\
            return ERR_RET;\
        }\
        catch(...){\
            Btk_SetError("Unknown Exception");\
            return ERR_RET;\
        }
#endif

#include <Btk/impl/scope.hpp>
#include <Btk/impl/core.hpp>
#include <Btk.hpp>
#include <Btk.h>

static thread_local std::string global_error;
static bool show_typerrror = true;//< default show typerrror
static void set_typeerror(const char *req,BtkWidget *widget){
    Btk_SetError("TypeError:(%s,%s)",req,Btk::get_typename(widget).c_str());
    if(show_typerrror){
        fputs(Btk_GetError(),stderr);
        fputc('\n',stderr);
    }
}
BTK_CAPI_BEGIN

//Global Init/Quit
bool Btk_Init(){
    BTK_BEGIN_CATCH();

    Btk::Init();
    return true;

    BTK_END_CATCH2(false);
}
int  Btk_Run(){
    return Btk::run();
}
void Btk_AtExit(Btk_callback_t callback,void *param){
    Btk::AtExit(callback,param);
}
void Btk_DeferCall(Btk_callback_t callback,void *param){
    Btk::DeferCall(callback,param);
}


//Widget

void Btk_UpdateRect(BtkWidget *widget,int x,int y,int w,int h){
    BTK_NUL_CHK(widget);
    widget->set_rect(x,y,w,h);
}
void Btk_AtDelete(BtkWidget *widget,Btk_callback_t fn,void*param){
    BTK_NUL_CHK(widget);
    widget->add_callback(fn,param);
}
void Btk_Delete(BtkWidget *widget){
    delete widget;
}




//Button
void Btk_AtButtonClicked(BtkButton *button,Btk_callback_t fn,void *param){
    BTK_NUL_CHK(button);
    BTK_NUL_CHK(fn);
    BTK_TYPE_CHK(button,Button);

    button->signal_clicked().connect([fn,param](){
        fn(param);
    });
}
BtkButton *Btk_NewButton(){
    return new BtkButton();
}
const char *Btk_SetButtonText(BtkButton *btn,const char *text){
    BTK_NUL_CHK2(btn,nullptr);
    BTK_TYPE_CHK2(btn,Button,nullptr);
    
    if(text != nullptr){
        btn->set_text(text);
    }
    return btn->text().data();
}

//TextBox
BtkTextBox *Btk_NewTextBox(){
    return new BtkTextBox();
}
void Btk_SetTextBoxText(BtkTextBox *textbox,const char *text){
    BTK_NUL_CHK(textbox);
    BTK_NUL_CHK(text);
    BTK_TYPE_CHK(textbox,TextBox);
    textbox->set_text(text);
}
char* Btk_GetTextBoxText(BtkTextBox *textbox){
    BTK_NUL_CHK2(textbox,nullptr);
    BTK_TYPE_CHK2(textbox,TextBox,nullptr);
    return Btk_strdup(textbox->u8text().c_str());
}
//Label
const char *Btk_SetLableText(BtkLabel *label,const char *text){
    BTK_NUL_CHK2(label,nullptr);
    if(text != nullptr){
        label->set_text(text);
    }
    return label->text().c_str();
}
//Window
BtkWindow *Btk_NewWindow(const char *title,int w,int h){
    return new Btk::Window(title,w,h);
}
void Btk_ShowWindow(BtkWindow *win){
    BTK_NUL_CHK(win);
    win->done();
}
void Btk_SetWindowTitle(BtkWindow *win,const char *title){
    BTK_NUL_CHK(win);
    win->set_title(title);
}
void Btk_SetWindowResizeable(BtkWindow *win,bool val){
    BTK_NUL_CHK(win);
    win->set_resizeable(val);
}
bool Btk_SetWindowIconFromFile(BtkWindow *win,const char *filename){
    BTK_NUL_CHK2(win,false);
    BTK_NUL_CHK2(filename,false);

    BTK_BEGIN_CATCH();

    win->set_icon(Btk::PixBuf::FromFile(filename));
    return true;
    BTK_END_CATCH2(false);
}
bool Btk_SetWindowCursorFromFile(BtkWindow *win,const char *filename){
    BTK_NUL_CHK2(win,false);
    BTK_NUL_CHK2(filename,false);

    BTK_BEGIN_CATCH();

    win->set_cursor(Btk::PixBuf::FromFile(filename));
    return true;
    BTK_END_CATCH2(false);
}
bool Btk_WindowAdd(BtkWindow*win,BtkWidget*widget){
    BTK_NUL_CHK2(win,false);
    BTK_NUL_CHK2(widget,false);

    return win->container().add(widget);
}
//ImageView
BtkImageView *Btk_NewImageView(){
    return new BtkImageView();
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
void Btk_Backtrace(){
    _Btk_Backtrace();
}
//Memory
void *Btk_malloc(size_t byte){
    void *ptr = malloc(byte);
    if(ptr == nullptr){
        Btk_SetError("malloc(%zu) failed",byte);
        return nullptr;
    }
    return ptr;
}
void *Btk_realloc(void *p,size_t byte){
    void *ptr = realloc(p,byte);
    if(ptr == nullptr){
        Btk_SetError("realloc(%p,%zu) failed",p,byte);
        return nullptr;
    }
    return ptr;
}
void  Btk_free(void *ptr){
    BTK_NUL_CHK(ptr);
    free(ptr);
}
char *Btk_strdup(const char *str){
    BTK_NUL_CHK2(str,nullptr);
    size_t len = strlen(str);
    char *ptr = static_cast<char*>(Btk_malloc((len + 1) * sizeof(char)));
    
    BTK_NUL_CHK2(ptr,nullptr);

    memcpy(ptr,str,(len + 1) * sizeof(char));
    return ptr;
}
//MessageBox
void Btk_MessageBox(const char *title,const char *message){
    Btk::MessageBox msgbox;
    msgbox.set_title(title);
    msgbox.set_message(message);
    msgbox.show();
}
//Signal
void Btk_SignConnect(BtkWidget *widget,const char *signal,...){
    BTK_NUL_CHK(widget);
    BTK_NUL_CHK(signal);
    va_list varg;
    va_start(varg,signal);
    Btk::Impl::VaListGuard vguard(varg);

    Btk_SetError("Unsupported");
}
//Format
size_t _Btk_impl_fmtargs(const char *fmt,...){
    if(fmt == nullptr){
        return 0;
    }
    size_t strsize;
    va_list varg;
    va_start(varg,fmt);
    #ifdef _WIN32
    strsize = _vscprintf(fmt,varg);
    #else
    strsize = vsnprintf(nullptr,0,fmt,varg);
    #endif
    va_end(varg);

    return (strsize + 1) * sizeof(char);
}
char* _Btk_impl_sprintf(char *buf,const char *fmt,...){
    if(buf == nullptr or fmt == nullptr){
        return nullptr;
    }
    va_list varg;
    va_start(varg,fmt);
    vsprintf(buf,fmt,varg);
    va_end(varg);
    return buf;
}
BTK_CAPI_END