//Widget impl
#include <Btk/impl/window.hpp>
#include <Btk/impl/widget.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/widget.hpp>
namespace Btk{
    WidgetImpl::~WidgetImpl(){

    }
};
namespace Btk{
    Widget::~Widget(){
        if(pimpl != nullptr){
            pimpl->unref();
        }
    }
    void Widget::draw(){
        //draw in render thread
        System::instance->defer_call([](void *__pimpl){
            static_cast<WidgetImpl*>(__pimpl)->draw();
        },pimpl);
    }
};