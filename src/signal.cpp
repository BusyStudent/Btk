#include "build.hpp"

#include <Btk/signal.hpp>
namespace Btk{
    #if 0
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
        for(auto iter = _connections.begin();iter != _connections.end();){
            //from ~HasSlots()
            iter->disconnect(true);
            iter = _connections.erase(iter);
        }
    }
    #endif
};