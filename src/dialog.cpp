#include "./build.hpp"

#include <Btk/impl/thread.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/dialog.hpp>

namespace Btk{
    Dialog::Dialog(){
        do_show.bind<&Dialog::default_show>();
    }
    Dialog::~Dialog() = default;
    auto Dialog::run() -> Status{
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
    void Dialog::default_show(){
        //Create a thread and exec
        Thread([this](){
            _event->clear();
            run();
            _event->set();
        }).detach();
    }
    void Dialog::show(){
        do_show(this);
    }
    //MessageBox
    MessageBox::~MessageBox(){
        _do_destroy();
    }
}