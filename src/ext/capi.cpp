#include "../build.hpp"


#define _BTK_CAPI_SOURCE

#define BTKC_NOEXCEPT_BEGIN() return _noexcept_call([&](){
#define BTKC_NOEXCEPT_END() });

#define BTKC_CHECK_PTR()
#define BTKC_CHECK_TYPE(PTR)

#include <typeindex>
#include <typeinfo>
#include <mutex>
#include <map>

#include <Btk/detail/window.hpp>
#include <Btk.hpp>
#include <Btk.h>

namespace{
    template<class T,class = void>
    struct _constructable:std::false_type{};
    template<class T>
    struct _constructable<T,
        std::void_t<
            decltype(new T())
        >>
        :std::true_type{

    };
    struct WidgetFactroy{
        std::map<std::type_index,BtkWidget (*)()> ctors;

        BtkWidget create(BtkType type) noexcept{
            std::type_index idx(*type);
            auto iter = ctors.find(idx);
            if(iter == ctors.end()){
                Btk_SetError("Couldnot create widget %s",BTK_typenameof(*type));
                return nullptr;
            }
            return iter->second();
        }
        template<class T>
        void register_type(){
            //Not virtual and can be constructable without args
            if constexpr(not std::is_abstract_v<T> and _constructable<T>::value){
                ctors[typeid(T)] = []() -> BtkWidget{
                    return new T;
                };
            }
        }
    };
    Btk::Constructable<WidgetFactroy> factory;
    thread_local Btk::u8string error_message;
    std::once_flag init_once;

    void _callback_forward(BtkWidget w,BtkCallback callback,void *param){
        callback(w,param);
    }
    auto _wrap_callback(BtkWidget w,BtkCallback callback,void *param){
        return Btk::Bind(_callback_forward,w,callback,param);
    }
    void _cleanup(){
        factory.destroy();
    }
    void _init(){
        factory.construct();
        Btk::AtExit(_cleanup);

        #define BTKC_DECLARE_WIDGET(T) factory->register_type<Btk::T>();
        BTKC_WIDGETS_LIST
        #undef BTKC_DECLARE_WIDGET
    }
    void _noexcept_call_impl(void (*callback)(void *param),void *p) noexcept{
        try{
            callback(p);
        }
        catch(std::exception &exp){
            Btk_SetError("%s",exp.what());
        }
        catch(...){
            Btk_SetError("Unknown Exception");
        }
    }
    void _noexcept_call_ret_impl(void (*callback)(void *param,void *pret),void *p,void *pret) noexcept{
        try{
            callback(p,pret);
        }
        catch(std::exception &exp){
            Btk_SetError("%s",exp.what());
        }
        catch(...){
            Btk_SetError("Unknown Exception");
        }
    }
    template<class Callable>
    std::invoke_result_t<Callable> _noexcept_call(Callable &&callable) noexcept{
        using RetT = std::invoke_result_t<Callable>;
        if constexpr(std::is_same_v<RetT,void>){
            _noexcept_call_impl([](void *cb){
                (*reinterpret_cast<Callable*>(cb))();
            },std::addressof(callable));
        }
        else{
            //Has return value
            RetT retvalue {};

            _noexcept_call_ret_impl([](void *cb,void *ret){
                (*reinterpret_cast<RetT*>(ret)) = (*reinterpret_cast<Callable*>(cb))();
            },std::addressof(callable),std::addressof(retvalue));

            return retvalue;
        }
    }
    void _on_nullptr() noexcept{
        Btk_SetError("Invalid nullptr param");
    }
    void _on_bad_type(BtkType req,BtkType except) noexcept{
        Btk_SetError("Require '%s' except '%s'",BTK_typenameof(*req),BTK_typenameof(*except));
    }

}

BTKC_API_BEGIN

#define BTKC_DECLARE_WIDGET(W) \
    BtkType BTKC_TYPEOF(Btk##W) = &typeid(Btk::W);
BTKC_WIDGETS_LIST
#undef BTKC_DECLARE_WIDGET


BtkWidget Btk_NewWidget(BtkType info){
    return factory->create(info);
}
void      Btk_Delete(BtkWidget widget){
    delete widget;
}
void      Btk_Init(){
    std::call_once(init_once,_init);
}

BtkString Btk_GetError(){
    return error_message.c_str();
}
void      Btk_SetError(BtkString fmt,...){
    std::va_list varg;
    va_start(varg,fmt);
    error_message.clear();
    error_message.append_vfmt(fmt,varg);
    va_end(varg);

}
void      Btk_ClearError(){
    error_message.clear();
}

//Type
BtkType   Btk_GetType(BtkWidget w){
    return &typeid(*w);
}
bool      Btk_TypeEqual(BtkType t1,BtkType t2){
    return *t1 == *t2;
}
BtkString Btk_TypeName(BtkType t){
    return t->name();
}


void      Btk_SetRectangle(BtkWidget wi,int x,int y,int w,int h){
    return _noexcept_call([&](){
        wi->set_rectangle(x,y,w,h);
    });
}
void      Btk_Hide(BtkWidget wi){
    return _noexcept_call([&](){
        wi->hide();
    });
}
void      Btk_Show(BtkWidget wi){
    return _noexcept_call([&](){
        wi->show();
    });
}
void      Btk_Resize(BtkWidget wi,int w,int h){
    return _noexcept_call([&](){
        wi->resize(w,h);
    });
}
//Container
bool      Btk_Add(BtkWidget parent,BtkWidget c){
    BTKC_NOEXCEPT_BEGIN();

    if(parent == nullptr or c == nullptr){
        _on_nullptr();
        return false;
    }
    if(not parent->is_container()){
        Btk_SetError("Require Container");
        return false;
    }
    return static_cast<BtkContainer>(parent)->add(c);

    BTKC_NOEXCEPT_END();
}
bool      Btk_Remove(BtkWidget parent,BtkWidget c){
    BTKC_NOEXCEPT_BEGIN();

    if(parent == nullptr or c == nullptr){
        _on_nullptr();
        return false;
    }
    if(not parent->is_container()){
        Btk_SetError("Require Container");
        return false;
    }
    return static_cast<BtkContainer>(parent)->remove(c);

    BTKC_NOEXCEPT_END();

}
bool      Btk_Detach(BtkWidget parent,BtkWidget c){
    BTKC_NOEXCEPT_BEGIN();

    if(parent == nullptr or c == nullptr){
        _on_nullptr();
        return false;
    }
    if(not parent->is_container()){
        Btk_SetError("Require Container");
        return false;
    }
    return static_cast<BtkContainer>(parent)->detach(c);

    BTKC_NOEXCEPT_END();
}

void      Btk_HookDestroy(BtkWidget w,BtkCallback cb,void *param){
    BTKC_NOEXCEPT_BEGIN();

    w->on_destroy(_wrap_callback(w,cb,param));
    
    BTKC_NOEXCEPT_END();
}


BtkWindow Btk_NewWindow(BtkString title,int x,int y){
    BTKC_NOEXCEPT_BEGIN();

    auto win = new Btk::Window(title,x,y);
    Btk_HookDestroy(win->impl(),[](BtkWidget,void *p){
        delete static_cast<BtkWindow>(p);
    },win);
    return win;

    BTKC_NOEXCEPT_END();
}
BtkWidget Btk_CastWindow(BtkWindow w){
    return w->impl();
}
bool      Btk_MainLoop(BtkWindow w){
    BTKC_NOEXCEPT_BEGIN();
    return w->mainloop();
    BTKC_NOEXCEPT_END();
}

//Btn
BtkString Btk_SetButtonText(BtkAbstractButton btn,BtkString txt){
    BTKC_NOEXCEPT_BEGIN();
    if(btn == nullptr){
        return (const char*)nullptr;
    }
    if(txt != nullptr){
        btn->set_text(txt);
    }
    return btn->text().data();
    BTKC_NOEXCEPT_END();
}
void     Btk_HookButtonClicked(BtkAbstractButton btn,BtkCallback cb,void *param){
    btn->signal_clicked().connect(_wrap_callback(btn,cb,param));
}
BTKC_API_END

