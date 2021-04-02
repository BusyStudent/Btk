#include "../build.hpp"

#include <Btk/msgbox/impl.hpp>
#include <Btk/async.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    MessageBox::~MessageBox(){
        if(pimpl != nullptr){
            pimpl->unref();
        }
    }
    MessageBox::MessageBox(std::string_view title,std::string_view txt,Flag flag){
        pimpl = new MessageBoxImpl();
        pimpl->title = title;
        pimpl->message = txt;
        pimpl->flag    = flag;
    }
    bool MessageBox::show(bool focus_async){
        ++(pimpl->refcount);
        if(focus_async or not CouldBlock()){
            Async(NoSignal,&MessageBoxImpl::Run,pimpl);
            return false;
        }
        else{
            pimpl->Run();
            return true;
        }
    }
    void MessageBox::set_title(std::string_view title){
        pimpl->title = title;
    }
    void MessageBox::set_message(std::string_view message){
        pimpl->message = message;
    }
    void MessageBox::set_flag(Flag flag){
        pimpl->flag = flag;
    }
    MessageBox::SignalAsync &MessageBox::signal_async(){
        return pimpl->signal;
    }
}