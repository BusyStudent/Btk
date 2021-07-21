#include "build.hpp"

#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_timer.h>
#include <Btk/object.hpp>
#include <Btk/Btk.hpp>
#include <atomic>

namespace Btk{
    Object::Object(){

    }
    Object::~Object(){
        cleanup();
    }
    void Object::cleanup(){
        lock_guard<Object> locker(*this);
        //cleanup all
        for(auto i = functors_cb.begin();i != functors_cb.end();){
            //Call the functor
            i->_call();
            i = functors_cb.erase(i);
        }
    }
    void Object::disconnect_all(){
        lock_guard<Object> locker(*this);
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
    _FunctorLocation Object::remove_callback_safe(FunctorLocation location){
        //Remove this callback after do check
        //Check is vaild location
        for(auto iter = functors_cb.begin(); iter != functors_cb.end(); ++iter){
            if(iter == location.iter){
                location->_cleanup();
                functors_cb.erase(location.iter);
                return {--functors_cb.end()};
            }
        }
        return {--functors_cb.end()};
    }
    void Object::dump_functors(FILE *output) const{
        if(output == nullptr){
            output = stderr;
        }
        lock_guard<const Object> locker(*this);
        for(auto &f:functors_cb){
            const char *type = nullptr;
            switch(f.magic){
                case Functor::Signal: type = "Signal";break;
                case Functor::Unknown: type = "Unknown";break;
                case Functor::Timer: type = "Timer";break;
            }
            fprintf(output,"  -(%s) => %p\n",type,f.call);
        }
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
    _GenericCallFunctor::_GenericCallFunctor(_GenericCallBase *base){
        call = [](_Functor &self){
            static_cast<_GenericCallBase*>(self.user1)->deleted = true;
        };
        cleanup = nullptr;
        user1 = base;
        magic = Unknown;
    }
}
namespace Btk{
    SignalBase::SignalBase(){

    }
    SignalBase::~SignalBase(){
        lock_guard<SignalBase> locker(*this);
        auto iter = slots.begin();
        while(iter != slots.end()){
            (*iter)->cleanup();
            iter = slots.erase(iter);
        }
    }
    void SignalBase::dump_slots(FILE *output) const{
        if(output == nullptr){
            output = stderr;
        }
        lock_guard<const SignalBase> locker(*this);
        for(auto slot:slots){
            fprintf(output,"  -(Slot => %p)\n",slot->cleanup_ptr);
        }
    }
    void Connection::disconnect(bool from_object){
        (*iter)->cleanup(from_object);
        current->slots.erase(iter);
    }
}
namespace Btk{
    _TimerFunctor::_TimerFunctor(Btk::Timer &timer,bool &need_remove_ref){
        call = [](_Functor &self){
            //Stop the timer and cleanup the invoker
            auto t = static_cast<Btk::Timer*>(self.user1);
            auto b = static_cast<bool*>(self.user2);
            *b = false;
            //Stop and reset invoker
            t->stop();
            t->reset();
        };
        cleanup = nullptr;

        user1 = &timer;
        user2 = &need_remove_ref;
        magic = Timer;
    }
    //Timer's impl
    struct BTKHIDDEN TimerImpl{
        ~TimerImpl(){
            cleanup_invoker();
        }

        Timer *owner = nullptr;
        SDL_TimerID timer_id = 0;
        bool running = false;
        
        _TimerInvokerData data;
        SpinLock data_spinlock;

        std::atomic<Uint32> interval = 0;

        Uint32 invoke(Uint32 new_interval);
        void   cleanup_invoker();
        static Uint32 SDLCALL Entry(Uint32,void*);
    };

    Uint32 TimerImpl::Entry(Uint32 cur_interval,void *self){
        BTK_LOGINFO("Timer timeout");
        return static_cast<TimerImpl*>(self)->invoke(cur_interval);
    }
    Uint32 TimerImpl::invoke(Uint32 new_interval){
        interval = new_interval;
        //Lock data begin call the invoker
        lock_guard<SpinLock> locker(data_spinlock);
        if(data.invoke == nullptr){
            return interval;
        }
        else{
            try{
                interval = data.invoke(interval,data.invoker);
            }
            catch(...){
                DeferCall(std::rethrow_exception,std::current_exception());
            }
        }
        Uint32 i = interval.load(std::memory_order::memory_order_acquire);
        if(i == 0){
            running = false;
            timer_id = 0;
        }
        return i;
    }
    void TimerImpl::cleanup_invoker(){
        lock_guard<SpinLock> locker(data_spinlock);

        if(data.invoker != nullptr){
            data.cleanup(data.invoker);

            data.invoker = nullptr;
            data.cleanup = nullptr;
            data.invoke = nullptr;
        }
    }
    void Timer::bind(_TimerInvokerData data){
        timer->cleanup_invoker();
        lock_guard<SpinLock> locker(timer->data_spinlock);
        timer->data = data;
    }
    void Timer::stop(){
        lock_guard<SpinLock> locker(timer->data_spinlock);
        if(timer->running){
            bool v = SDL_RemoveTimer(timer->timer_id);
            timer->running = false;
            timer->timer_id = 0;
        }
    }
    void Timer::start(){
        lock_guard<SpinLock> locker(timer->data_spinlock);
        if(not timer->running){
            timer->running = true;
            timer->timer_id = SDL_AddTimer(
                timer->interval,
                TimerImpl::Entry,
                timer
            );
        }
    }
    void Timer::set_interval(Uint32 interval){
        timer->interval = interval;
    }
    Uint32 Timer::interval() const{
        return timer->interval;
    }
    bool Timer::running() const{
        return timer->running;
    }
    Timer::Timer(){
        timer = new TimerImpl;
    }
    Timer::~Timer(){
        stop();
        delete timer;
    }
}