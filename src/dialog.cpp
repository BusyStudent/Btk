#include "./build.hpp"

#include <Btk/impl/thread.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/dialog.hpp>

namespace Btk{
    Dialog::~Dialog() = default;
    auto Dialog::show() -> Status{
        if(_opened){
            //Double call
            return _status;
        }


        _opened = true;
        
        _status = do_run(this);
        //Emit signal
        _signal_finished();
        if(_status == Accepted){
            _signal_accepted();
        }
        else{
            _signal_rejected();
        }

        _opened = false;
        return _status;
    }
    auto Dialog::wait() -> Status{
        if(_asynced){
            _event->wait();
        }
        return _status;
    }
    void Dialog::async_entry(){
        _event->clear();
        show();
        _event->set();
    }
    void Dialog::async(){
        Thread(&Dialog::async_entry,this).detach();
    }
    //Normal box
    MessageBox::MessageBox(u8string_view t,u8string_view m,Flag f){
        _title = t;
        _message = m;
        _flag = f;

        register_run_fn<&MessageBox::_do_run>();
    }
}