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
    void Object::cleanup(){
        std::lock_guard<Object> locker(*this);
        //cleanup all
        for(auto i = functors_cb.begin();i != functors_cb.end();){
            //Call the functor
            i->_call();
            i = functors_cb.erase(i);
        }
    }
    void Object::disconnect_all(){
        std::lock_guard<Object> locker(*this);
        //disconnect the timer
        for(auto i = functors_cb.begin();i != functors_cb.end();){
            //Call the functor
            if(i->magic == Functor::Signal){
                i->_call();
                i = functors_cb.erase(i);
            }
            else{
                ++i;
            }
        }
    }
    _FunctorLocation Object::add_callback(void(*fn)(void*),void*param){
        if(fn == nullptr){
            return {functors_cb.end()};
        }
        Functor functor;
        functor.user1 = reinterpret_cast<void*>(fn);
        functor.user2 = param;
        functor.call = [](Functor&self) -> void{
            reinterpret_cast<void(*)(void*)>(self.user1)(self.user2);
        };

        functors_cb.push_back(functor);
        return {--functors_cb.end()};
    }
    _FunctorLocation Object::add_functor(const Functor &functor){
        functors_cb.push_back(functor);
        return {--functors_cb.end()};
    }
    _FunctorLocation Object::remove_callback(FunctorLocation location){
        //Remove this callback
        if(location.iter != functors_cb.end()){
            location->_cleanup();
            functors_cb.erase(location.iter);
        }
        return {--functors_cb.end()};
    }
}
namespace Btk{
    //Remove the 
    _ConnectionFunctor::_ConnectionFunctor(Connection con){
        //Execute the functor
        call = [](_Functor &self){
            auto *con = static_cast<Connection*>(self.user1);
            con->disconnect(true);
            delete con;
        };
        //Just cleanup
        cleanup = [](_Functor &self){
            auto *con = static_cast<Connection*>(self.user1);
            delete con;
        };

        user1 = new Connection(con);
        magic = Signal;
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