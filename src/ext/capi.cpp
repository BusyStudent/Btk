#include "../build.hpp"


#define _BTK_CAPI_SOURCE
#define BTK_CAPI_BEGIN extern "C"{
#define BTK_CAPI_END }

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
    template<class Callable>
    std::invoke_result_t<Callable> _noexcept_call(Callable &&callable) noexcept{
        _noexcept_call_impl([](void *cb){
            (*reinterpret_cast<Callable*>(cb))();
        },std::addressof(callable));
    }
}

BTK_CAPI_BEGIN

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

void      Btk_HookDestroy(BtkWidget w,BtkCallback cb,void *param){
    w->on_destroy(_wrap_callback(w,cb,param));
}


BtkWindow Btk_NewWindow(BtkString title,int x,int y){
    auto win = new Btk::Window(title,x,y);
    Btk_HookDestroy(win->impl(),[](BtkWidget,void *p){
        delete static_cast<BtkWindow>(p);
    },win);
    return win;
}
BtkWidget Btk_CastWindow(BtkWindow w){
    return w->impl();
}

BTK_CAPI_END
