#include "build.hpp"

#include <SDL2/SDL_atomic.h>
#include <Btk/object.hpp>

namespace Btk{
    Object::Object(){

    }
    Object::~Object(){
        auto iter = callbacks.begin();
        while(iter != callbacks.end()){
            CallBack *cb = *iter;
            if(cb != nullptr){
                //Call the callback
                cb->run(this,cb);
            }
            iter = callbacks.erase(iter);
        }
    }
    //multithreading
    void Object::lock() const{
        SDL_AtomicLock(&spinlock);
    }
    void Object::unlock() const{
        SDL_AtomicUnlock(&spinlock);
    }
    void Object::disconnect_all(){
        auto iter = callbacks.begin();
        while(iter != callbacks.end()){
            CallBack *cb = *iter;
            if(cb->type == CallBack::Signal){
                //Call the callback
                cb->run(this,cb);
            }
            iter = callbacks.erase(iter);
        }
    }
}
namespace Btk{
    void SignalBase::lock() const{
        SDL_AtomicLock(&spinlock);
    }
    void SignalBase::unlock() const{
        SDL_AtomicUnlock(&spinlock);
    }
    SignalBase::SignalBase(){

    }
    SignalBase::~SignalBase(){
        lock_guard locker(this);
        auto iter = slots.begin();
        while(iter != slots.end()){
            (*iter)->cleanup();
            iter = slots.erase(iter);
        }
    }
}