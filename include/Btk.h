#if !defined(_BTK_CAPI_H_)
#define _BTK_CAPI_H_

#ifdef __cplusplus
    #define BTKC_API_BEGIN extern "C"{
    #define BTKC_API_END }
#else
    #define BTKC_API_BEGIN 
    #define BTKC_API_END 
#endif


#ifdef _WIN32
    #ifdef _MSC_VER
    #define BTKC_EXPORT __declspec(dllexport)
    #define BTKC_IMPORT __declspec(dllimport)
    #else
    #define BTKC_EXPORT __attribute__((dllexport))
    #define BTKC_IMPORT __attribute__((dllimport))
    #endif
#elif defined(__GNUC__)
    #define BTKC_EXPORT __attribute__((visibility("default"))) 
    #define BTKC_IMPORT 
#else
    #define BTKC_EXPORT 
    #define BTKC_IMPORT 
#endif


#ifdef _BTK_CAPI_SOURCE
    #define BTKC_API BTKC_EXPORT
#else
    #define BTKC_API
#endif

//Headers
#include <stdbool.h>
#include <stddef.h>

//Type
#ifdef _BTK_CAPI_SOURCE
typedef const       std::type_info* BtkType;
typedef struct      SDL_Surface   * BtkPixBuf;
typedef             Btk::Widget   * BtkWidget;
typedef             Btk::Window   * BtkWindow;
#else
typedef const struct _Btk_typeinfo* BtkType;
typedef struct       _BtkPixBuf   * BtkPixBuf;
typedef struct       _BtkWidget   * BtkWidget;
typedef struct       _BtkWindow   * BtkWindow;
#endif
typedef const        char         * BtkString;
typedef void                     (* BtkCallback)(BtkWidget,void*);
typedef void                     (* BtkCallback1)(BtkWidget);
typedef void                     (* BtkCallback0)();

typedef BtkPixBuf BtkBitmap;

#define BTKC_DECLARE_TYPE(TYPE) \
    extern BtkType BTKC_TYPEOF(TYPE)
#define BTKC_TYPEOF(TYPE) _##TYPE##_t


#define BTKC_DECLARE_WIDGET_EXTERNAL(WIDGET) \
    typedef BtkWidget Btk##WIDGET;\
    BTKC_DECLARE_TYPE(Btk##WIDGET);\
    static inline Btk##WIDGET Btk_New##WIDGET(){\
        return (Btk##WIDGET)Btk_NewWidget(BTKC_TYPEOF(Btk##WIDGET));\
    }
#define BTKC_DECLARE_WIDGET_INTERNAL(WIDGET) \
    typedef Btk::WIDGET *Btk##WIDGET;\
    BTKC_DECLARE_TYPE(Btk##WIDGET);\
    static inline Btk##WIDGET Btk_New##WIDGET(){\
        return (Btk##WIDGET)Btk_NewWidget(BTKC_TYPEOF(Btk##WIDGET));\
    }
#define BTKC_DECLARE_WIDGET_EXTERNAL_STIRCT(WIDGET) \
    typedef struct _Btk##WIDGET *Btk##WIDGET;\
    BTKC_DECLARE_TYPE(Btk##WIDGET);\
    static inline Btk##WIDGET Btk_New##WIDGET(){\
        return (Btk##WIDGET)Btk_NewWidget(BTKC_TYPEOF(Btk##WIDGET));\
    }
#define BTKC_WIDGETS_LIST \
    BTKC_DECLARE_WIDGET(Button) \
    BTKC_DECLARE_WIDGET(RadioButton) \
    BTKC_DECLARE_WIDGET(AbstractButton) \
    BTKC_DECLARE_WIDGET(Layout) \
    BTKC_DECLARE_WIDGET(BoxLayout) \
    BTKC_DECLARE_WIDGET(HBoxLayout) \
    BTKC_DECLARE_WIDGET(VBoxLayout) \
    BTKC_DECLARE_WIDGET(Container) \
    BTKC_DECLARE_WIDGET(Group) \
    BTKC_DECLARE_WIDGET(ImageView) \

BTKC_API_BEGIN

BTKC_API int       Btk_Run();
BTKC_API void      Btk_Init();

BTKC_API BtkWidget Btk_NewWidget(BtkType type);
BTKC_API BtkType   Btk_GetType(BtkWidget widget);
BTKC_API void      Btk_Delete(BtkWidget widget);

BTKC_API bool      Btk_TypeBefore(BtkType t1,BtkType t2);
BTKC_API bool      Btk_TypeEqual(BtkType t1,BtkType t2);
BTKC_API size_t    Btk_TypeHash(BtkType t);
BTKC_API BtkString Btk_TypeName(BtkType t);


BTKC_API BtkString Btk_GetError();
BTKC_API void      Btk_SetError(BtkString fmt,...);
BTKC_API void      Btk_ClearError();



BTKC_API void      Btk_SetRectangle(BtkWidget wi,int x,int y,int w,int h);
BTKC_API void      Btk_Resize(BtkWidget wi,int w,int h);
BTKC_API void      Btk_Show(BtkWidget wi);
BTKC_API void      Btk_Hide(BtkWidget wi);

BTKC_API bool      Btk_Add(BtkWidget parent,BtkWidget c);
BTKC_API bool      Btk_Remove(BtkWidget parent,BtkWidget c);
BTKC_API bool      Btk_Detach(BtkWidget parent,BtkWidget c);

BTKC_API void      Btk_HookDestroy(BtkWidget w,BtkCallback cb,void *p);

//Window
BTKC_API BtkWindow Btk_NewWindow(BtkString title,int x,int y);
BTKC_API BtkWidget Btk_CastWindow(BtkWindow win);
BTKC_API bool      Btk_MainLoop(BtkWindow win);
//Window Method
BTKC_API void      Btk_SetWindowTitle(BtkWindow win,BtkString title);
//Expose widget
#if defined(_BTK_CAPI_SOURCE) && !defined(_BTKC_STRICT)
    #define BTKC_DECLARE_WIDGET BTKC_DECLARE_WIDGET_INTERNAL
#elif !defined(BTKC_STRICT)
    #define BTKC_DECLARE_WIDGET BTKC_DECLARE_WIDGET_EXTERNAL
#else
    #define BTKC_DECLARE_WIDGET BTKC_DECLARE_WIDGET_EXTERNAL_STRICT
#endif

    BTKC_WIDGETS_LIST

#undef BTKC_DECLARE_WIDGET


//Widgets Method

//Button Method
BTKC_API BtkString Btk_SetButtonText(BtkAbstractButton btn,BtkString txt);
BTKC_API void      Btk_HookButtonClicked(BtkAbstractButton btn,BtkCallback cb,void *p);
//ImageView
BTKC_API void      Btk_SetViewImage(BtkImageView view,BtkPixBuf bitmap);


BTKC_API_END

#endif // _BTK_CAPI_H_
