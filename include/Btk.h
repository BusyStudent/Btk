#if !defined(_BTK_CAPI_H_)
#define _BTK_CAPI_H_

#ifdef __cplusplus
    #define BTK_CLINKAGE extern "C"
#else
    #define BTK_CLINKAGE
#endif


#ifdef _WIN32
    #ifdef _MSC_VER
    #define BTK_C_EXPORT __declspec(dllexport)
    #define BTK_C_IMPORT __declspec(dllimport)
    #else
    #define BTK_C_EXPORT __attribute__((dllexport))
    #define BTK_C_IMPORT __attribute__((dllimport))
    #endif
#elif defined(__GNUC__)
    #define BTK_C_EXPORT __attribute__((visibility("default"))) 
    #define BTK_C_IMPORT 
#else
    #define BTK_C_EXPORT 
    #define BTK_C_IMPORT 
#endif

#ifdef _BTK_SOURCE
    #define BTK_CAPI BTK_CLINKAGE BTK_C_EXPORT
    #define BTK_DEF_WIDGET(NAME,SUPER) \
        typedef Btk::NAME Btk##NAME;\
        BTK_CAPI bool Btk_##Is##NAME(BtkWidget *widget)\
        {\
            return dynamic_cast<Btk##NAME*>(\
                reinterpret_cast<Btk::Widget*>(widget)\
            ) != nullptr;\
        }
#else
    #define BTK_CAPI BTK_CLINKAGE BTK_C_IMPORT
    //Define Widget and function
    #ifdef __cplusplus
        #define BTK_DEF_WIDGET(NAME,SUPER) \
        typedef struct Btk##NAME:public Btk##SUPER{}Btk##NAME;\
        BTK_CAPI bool Btk_Is##NAME(BtkWidget *);\
        BTK_CAPI Btk##NAME* Btk_New##NAME();

    //strict modes
    #elif defined(BTK_STRICT)
        #define BTK_DEF_WIDGET(NAME,SUPER) \
            typedef struct Btk##NAME Btk##NAME;\
            BTK_CAPI bool Btk_Is##NAME(BtkWidget *);\
            BTK_CAPI Btk##NAME* Btk_New##NAME();
    #else
        #define BTK_DEF_WIDGET(NAME,SUPER) \
        typedef BtkWidget Btk##NAME;\
        BTK_CAPI bool Btk_Is##NAME(BtkWidget *);\
        BTK_CAPI Btk##NAME* Btk_New##NAME();
    #endif
#endif
//name begin
#if !defined(__cplusplus)
typedef struct BtkWindow BtkWindow;
typedef struct BtkWidget BtkWidget;
typedef struct BtkContainer BtkContainer;
#include <stdbool.h>
#elif defined(_BTK_SOURCE)
#include <Btk/window.hpp>
#include <Btk/widget.hpp>
typedef Btk::Window BtkWindow;
typedef Btk::Widget BtkWidget;
typedef Btk::Container BtkContainer;
#else
struct BtkWindow{};
struct BtkWidget{};
struct BtkContainer{};
#endif

typedef struct BtkPixBuf BtkPixBuf;
typedef BtkPixBuf BtkBitmap;

//stack alloc
#ifdef _WIN32
    #include <malloc.h>
    #define Btk_alloca(S) _alloca(S)
#elif defined(__linux)
    #include <alloca.h>
    #define Btk_alloca(S) alloca(S)
#elif defined(__GNUC__)
    #define Btk_alloca(S) __builtin_alloca(S)
#else
    #define Btk_alloca(S) NULL
#endif

//format string at stack memory
#define Btk_format(...) _Btk_impl_sprintf(\
    (char*)Btk_alloca(_Btk_impl_fmtargs(__VA_ARGS__)),\
    __VA_ARGS__)
#define Btk_UNUSED(P) (void)P

//headers
#include <stddef.h>
#include <stdint.h>

//name casttinh macro
#define BTK_WIDGET(PTR) ((BtkWidget*)PTR)
//name alias
#define Btk_delete Btk_Delete
#define Btk_run    Btk_Run
#define Btk_SetWidgetRect Btk_UpdateRect
#define Btk_SetRect Btk_UpdateRect
/**
 * @brief Generic callback
 * 
 */
typedef void(*Btk_callback_t)(void*);
typedef void(*Btk_ccallback_t)();

//Buttons
BTK_DEF_WIDGET(AbstractButton,Widget);
BTK_DEF_WIDGET(Button,AbstractButton);
//TextBox
BTK_DEF_WIDGET(TextBox,Widget);
//Label
BTK_DEF_WIDGET(Label,Widget);
//ImageView
BTK_DEF_WIDGET(ImageView,Widget);
BTK_DEF_WIDGET(ScrollBar,Widget);

#ifndef BTK_NGIF
BTK_DEF_WIDGET(GifView,Widget);
#endif
//name end

//function begin
BTK_CAPI bool Btk_Init();
BTK_CAPI int  Btk_Run();
BTK_CAPI void Btk_AtExit(Btk_callback_t,void *param);
BTK_CAPI void Btk_Async(Btk_callback_t,void *param);
BTK_CAPI void Btk_DeferCall(Btk_callback_t,void *param);
//Memory
BTK_CAPI void *Btk_malloc(size_t byte);
BTK_CAPI void *Btk_realloc(void *ptr,size_t byte);
BTK_CAPI void  Btk_free(void *ptr);
BTK_CAPI char *Btk_strdup(const char *str);
//widgets
BTK_CAPI void Btk_SetWidgetPosition(BtkWidget *,int x,int y);
/**
 * @brief Update Widget's rect
 * 
 * @param widget
 * @param x 
 * @param y 
 * @param w 
 * @param h 
 * @return BTK_CAPI 
 */
BTK_CAPI void Btk_UpdateRect(BtkWidget *,int x,int y,int w,int h);
/**
 * @brief Register callback at widget's destruction
 * 
 * @return BTK_CAPI 
 */
BTK_CAPI void Btk_AtDelete(BtkWidget *,Btk_callback_t,void*);
BTK_CAPI void Btk_Delete(BtkWidget *widget);
//Button
BTK_CAPI void Btk_AtButtonClicked(BtkButton *btn,Btk_callback_t,void*);
/**
 * @brief Set button text
 * 
 * @param btn The button pointer
 * @param text The new text(nullptr on no-op)
 * @return The current Button's text  
 */
BTK_CAPI const char *Btk_SetButtonText(BtkButton *btn,const char *text);
#define Btk_GetButtonText(BTN) Btk_SetButtonText(BTN,NULL)

//TextBox
BTK_CAPI void  Btk_SetTextBoxText(BtkTextBox *textbox,const char *text);
/**
 * @brief Get the text Your shoud free the string by Btk_free
 * 
 * @param textbox 
 * @return char *
 */
BTK_CAPI char *Btk_GetTextBoxText(BtkTextBox *textbox);
//Label
BTK_CAPI const char *Btk_SetLableText(BtkLabel*,const char *text);
#define Btk_GetLableText(L) Btk_SetLableText(L,NULL)
//Signal
/**
 * @brief Connect signal
 * 
 * @param signal The signal name
 * @param ... 
 * @return BTK_CAPI 
 */
BTK_CAPI void Btk_SignConnect(BtkWidget*,const char *signal,...);
//Widget
typedef bool (*Btk_foreach_t)(BtkWidget *widget,void *userdata);
BTK_CAPI void Btk_ForChildrens(BtkWidget *w,Btk_foreach_t fn,void *p);
//PixBuf
BTK_CAPI BtkPixBuf *Btk_LoadImage(const char *filename);
BTK_CAPI void Btk_FreeImage(BtkPixBuf *);
//window
BTK_CAPI BtkWindow *Btk_NewWindow(const char *title,int w,int h);
BTK_CAPI void Btk_ShowWindow(BtkWindow *);
BTK_CAPI void Btk_SetWindowTitle(BtkWindow*,const char *title);
BTK_CAPI void Btk_SetWindowResizeable(BtkWindow*,bool resizeable);
/**
 * @brief Add a widget into a window
 * 
 * @return BTK_CAPI 
 */
BTK_CAPI bool Btk_WindowAdd(BtkWindow*,BtkWidget*);
/**
 * @brief Show the window and enter the eventloop
 * 
 * @return BTK_CAPI 
 */
BTK_CAPI bool Btk_MainLoop(BtkWindow*);
/**
 * @brief Set the window icon from filename
 * 
 * @param filename 
 * @return BTK_CAPI 
 */
BTK_CAPI bool Btk_SetWindowIconFromFile(BtkWindow*,const char *filename);
BTK_CAPI bool Btk_SetWindowCursorFromFile(BtkWindow*,const char *filename);
//Gif
#ifndef BTK_NGIF
BTK_CAPI bool Btk_SetGifViewImageFrom(BtkGifView *view,const char *filename);
#endif
//Error
BTK_CAPI const char *Btk_GetError();
BTK_CAPI void        Btk_SetError(const char *fmt,...);
//MessageBox
BTK_CAPI void Btk_MessageBox(const char *title,const char *message);
//Debug
BTK_CAPI char *Btk_typeof(BtkWidget *);
/**
 * @brief Dymaic Cast 
 * 
 */
#define Btk_dymaic_cast(TYPE,POINTER) \
    (Btk##TYPE*)_Btk_dymaic_cast(Btk_Is##TYPE,POINTER)
inline BtkWidget *_Btk_dymaic_cast(bool(*check)(BtkWidget*),BtkWidget* w){
    if(check(w)){
        return w;
    }
    return NULL;
}
//function end

//hidden api begin
/**
 * @brief Return the size of the buffer
 * 
 * @param fmt 
 * @param ... 
 * @return BTK_CAPI 
 */
BTK_CAPI size_t _Btk_impl_fmtargs(const char *fmt,...);
/**
 * @brief format the string into buffer
 * 
 * @param buf 
 * @param fmt 
 * @param ... 
 * @return The buf ptr
 */
BTK_CAPI char*  _Btk_impl_sprintf(char *buf,const char *fmt,...);
//hidden api end
#endif // _BTK_CAPI_H_
