#include "../build.hpp"
#include <Btk/msgbox/impl.hpp>
#include <Btk/async.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    FSelectBox::~FSelectBox(){
        if(pimpl != nullptr){
            pimpl->unref();
        }
    }
    FSelectBox::FSelectBox(const FSelectBox &box){
        pimpl = box.pimpl;
        if(box.pimpl != nullptr){
             ++(pimpl->refcount);
        }
    }
    FSelectBox::FSelectBox(u8string_view title){
        pimpl = new FSelectBoxImpl;
        pimpl->title = title;
    }
    bool FSelectBox::show(bool focus_async){
        ++(pimpl->refcount);
        if(focus_async or not CouldBlock()){
            Async(NoSignal,&FSelectBoxImpl::Run,pimpl).lauch();
            return false;
        }
        else{
            pimpl->Run();
            return true;
        }
    }
    void FSelectBox::set_multi(bool val){
        pimpl->multiple = val;
        if(val){
            pimpl->save = false;
        }
    }
    void FSelectBox::set_save(bool val){
        pimpl->save = val;
        if(val){
            //The multipie flag was conflict with it
            pimpl->multiple = false;
        }
    }
    FSelectBox::SignalAsync &FSelectBox::signal_async(){
        return pimpl->signal;
    }

}
