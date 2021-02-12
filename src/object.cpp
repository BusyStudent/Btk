#include "build.hpp"

#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_timer.h>
#include <Btk/object.hpp>

namespace Btk{
    Object::Object(){

    }
    Object::~Object(){
        cleanup();
    }
    //multithreading
    void Object::lock() const{
        SDL_AtomicLock(&spinlock);
    }
    void Object::unlock() const{
        SDL_AtomicUnlock(&spinlock);
    }
    void Object::disconnect_all(){
        lock_guard locker(this);

        auto iter = callbacks.begin();
        while(iter != callbacks.end()){
            auto *cb = *iter;
            if(cb->type == ObjectCallBack::Signal){
                //Call the callback
                cb->run(this,cb);
            }
            iter = callbacks.erase(iter);
        }
    }
    void Object::cleanup(){
        lock_guard locker(this);

        auto iter = callbacks.begin();
        while(iter != callbacks.end()){
            auto *cb = *iter;
            if(cb != nullptr){
                //Call the callback
                cb->run(this,cb);
            }
            iter = callbacks.erase(iter);
        }
    }
    void ConnectionWrapper::Run(Object *object,ObjectCallBack *self){
        std::unique_ptr<ConnectionWrapper> ptr(
            static_cast<ConnectionWrapper*>(self)
        );
        //if object is nullptr => do nothing
        if(object != nullptr){
            //disconnect it
            ptr->con.disconnect(true);
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