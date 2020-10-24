#include <Btk/signal/signal.hpp>
namespace Btk{
    SignalBase::SignalBase(){
        
    }
    SignalBase::~SignalBase(){
        disconnect_all();
    }

    HasSlots::HasSlots(){

    }
    HasSlots::~HasSlots(){
        disconnect_all();
    }
    void HasSlots::disconnect_all(){
        for(auto c:_connections){
            //from ~HasSlots()
            c.disconnect(true);
        }
    }
};