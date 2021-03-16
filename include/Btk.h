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
        BTK_CAPI bool Btk_Is##NAME(BtkWidget *);

    //strict modes
    #elif defined(BTK_STRICT)
        #define BTK_DEF_WIDGET(NAME,SUPER) \
            typedef struct Btk##NAME Btk##NAME;\
            BTK_CAPI bool Btk_Is##NAME(BtkWidget *);
    #else
        #define BTK_DEF_WIDGET(NAME,SUPER) \
        typedef BtkWidget Btk##NAME;\
        BTK_CAPI bool Btk_Is##NAME(BtkWidget *);
    #endif
#endif
//name begin
#if !defined(__cplusplus)
typedef struct BtkWindow BtkWindow;
typedef struct BtkWidget BtkWidget;
#include <stdbool.h>
#elif defined(_BTK_SOURCE)
#include <Btk/window.hpp>
#include <Btk/widget.hpp>
typedef Btk::Window BtkWindow;
typedef Btk::Widget BtkWidget;
#else
struct BtkWindow{};
struct BtkWidget{};
#endif
//name casttinh macro
#define BTK_WIDGET(PTR) ((BtkWidget*)PTR)
//name alias
#define Btk_delete Btk_Delete
#define Btk_run    Btk_Run
#define Btk_SetWidgetRect Btk_UpdateRect
//Buttons
BTK_DEF_WIDGET(AbstructButton,Widget);
BTK_DEF_WIDGET(Button,AbstructButton);
//TextBox
BTK_DEF_WIDGET(TextBox,Widget);
//name end

//function begin
BTK_CAPI bool Btk_Init();
BTK_CAPI int  Btk_Run();
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
BTK_CAPI void Btk_AtDelete(BtkWidget *,void(*)(void*),void*);
BTK_CAPI void Btk_Delete(BtkWidget *widget);

//window
BTK_CAPI BtkWindow *Btk_NewWindow(const char *title,int w,int h);
//Error
BTK_CAPI const char *Btk_GetError();
//function end
#endif // _BTK_CAPI_H_
