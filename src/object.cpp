#include "build.hpp"

#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_timer.h>
#include <Btk/exception.hpp>
#include <Btk/object.hpp>
#include <Btk/Btk.hpp>
#include <atomic>

namespace Btk{
    Object::Object(){

    }
    Object::~Object(){
        if(_impl != nullptr){
            _impl->cleanup();
            delete _impl;
        }
    }
    void Object::Impl::cleanup(){
        lock_guard<SpinLock> locker(spinlock);
        //cleanup all
        for(auto i = functors_cb.begin();i != functors_cb.end();){
            //Call the functor
            i->_call();
            i = functors_cb.erase(i);
        }
    }
    void Object::Impl::disconnect_all(){
        lock_guard<SpinLock> locker(spinlock);
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
    _FunctorLocation Object::Impl::add_callback(void(*fn)(void*),void*param){
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
    _FunctorLocation Object::Impl::add_functor(const Functor &functor){
        functors_cb.push_back(functor);
        return {--functors_cb.end()};
    }
    _FunctorLocation Object::Impl::remove_callback(FunctorLocation location){
        //Remove this callback
        if(location.iter != functors_cb.end()){
            location->_cleanup();
            functors_cb.erase(location.iter);
        }
        return {--functors_cb.end()};
    }
    _FunctorLocation Object::Impl::exec_functor(FunctorLocation location){
        //Remove this callback
        if(location.iter != functors_cb.end()){
            location->_call();
            functors_cb.erase(location.iter);
        }
        return {--functors_cb.end()};
    }
    _FunctorLocation Object::Impl::remove_callback_safe(FunctorLocation location){
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
    void Object::Impl::dump_functors(FILE *output) const{
        if(output == nullptr){
            output = stderr;
        }
        lock_guard<SpinLock> locker(spinlock);
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
    Uint32 Object::GetTicks(){
        return SDL_GetTicks();
    }
    auto Object::impl() const -> Impl&{
        if(_impl == nullptr){
            _impl = new Impl;
        }
        return *_impl;
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
        disconnect_all();
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
    void SignalBase::disconnect_all(){
        lock_guard<SignalBase> locker(*this);
        auto iter = slots.begin();
        while(iter != slots.end()){
            (*iter)->cleanup();
            iter = slots.erase(iter);
        }
    }
    void Connection::disconnect(bool from_object){
        if(status == WithSignal){
            (*sig.iter)->cleanup(from_object);
            sig.current->slots.erase(sig.iter);
        }
        else if(status == WithObject){
            obj.object->exec_functor(obj.loc);
        }
        status = None;
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
        inline
        TimerImpl(){

        }
        inline
        ~TimerImpl(){
            while(running){
                //Wait for the callback
                SDL_Delay(1);
            }
            stop();
            reset_callback();
        }

        SDL_threadID thread_id = {};
        SDL_TimerID timer_id = 0;
        bool running = false;
        
        _TimerInvokerData callback;
        SpinLock spinlock;

        std::atomic<Uint32> interval = 0;

        //Method
        inline
        void stop();

        inline
        void start();

        inline
        void set_interval(Uint32 interval);

        inline
        Uint32 get_interval();

        inline
        Uint32 entry(Uint32 new_interval);
        inline
        void   reset_callback();
        inline
        void   set_callback(const _TimerInvokerData *);
        static Uint32 SDLCALL Entry(Uint32,void*);
    };
    Uint32 TimerImpl::Entry(Uint32 cur_interval,void *self){
        BTK_LOGINFO("[Timer]Timeout %p",self);
        return static_cast<TimerImpl*>(self)->entry(cur_interval);
    }
    inline
    void TimerImpl::set_callback(const _TimerInvokerData *data){
        reset_callback();
        if(data != nullptr){
            callback = *data;
        }
    }
    inline
    void TimerImpl::reset_callback(){
        if(callback.destroy != nullptr){
            callback.destroy(callback.self);
            callback.destroy = nullptr;
        }
    }
    inline
    Uint32 TimerImpl::entry(Uint32 cur_interval){
        thread_id = SDL_ThreadID();
        lock_guard<SpinLock> locker(spinlock);
        running = true;
        //Lock succeed
        if(interval == 0){
            //The timer was canceled
        }
        else if(callback.entry == nullptr){
            //Null callback
            interval = cur_interval;
        }
        else{
            try{
                interval = callback.entry(cur_interval,callback.self);
            }
            catch(...){
                DeferCall(std::rethrow_exception,std::current_exception());
            }
        }
        //end
        running = false;
        if(interval == 0){
            timer_id = 0;
            BTK_LOGINFO("[Timer]Stop %p",this);
        }
        return interval;
    }
    inline
    void  TimerImpl::set_interval(Uint32 interval){
        if(thread_id == SDL_ThreadID()){
            //Is the timer thread,no lock needed
            this->interval = interval;
        }
        else if(timer_id == 0){
            //No started
            this->interval = interval;
        }
        else{
            stop();
            this->interval = interval;
            start();
        }
    }
    inline
    Uint32 TimerImpl::get_interval(){
        if(thread_id == SDL_ThreadID()){
            //No lock needed
            return interval;
        }
        lock_guard<SpinLock> locker(spinlock);
        return interval;
    }
    inline
    void  TimerImpl::start(){
        if(timer_id == 0){
            BTK_LOGINFO("[Timer]Start %p,interval %u",this,interval.load());
            timer_id = SDL_AddTimer(
                interval,
                Entry,
                this
            );
            if(timer_id == 0){
                //Create failed
                throwSDLError();
            }
        }
    }
    inline
    void  TimerImpl::stop(){
        if(timer_id != 0){
            BTK_LOGINFO("[Timer]Stop %p",this);
            
            bool v = SDL_RemoveTimer(timer_id);
            timer_id = 0;

            if(not v){
                //Failed
                BTK_LOGWARN("[Timer]failed to stop timer => %s",SDL_GetError());
            }
        }
    }
    void * Timer::_New(){
        return new TimerImpl();
    }
    void   Timer::_Stop(void *timer){
        static_cast<TimerImpl*>(timer)->stop();
    }
    void   Timer::_Start(void *timer){
        static_cast<TimerImpl*>(timer)->start();
    }
    void   Timer::_Delete(void *timer){
        delete static_cast<TimerImpl*>(timer);
    }
    void   Timer::_SetInterval(void *timer,Uint32 interval){
        static_cast<TimerImpl*>(timer)->set_interval(interval);;
    }
    Uint32 Timer::_GetInterval(void *timer){
        return static_cast<TimerImpl*>(timer)->get_interval();
    }
    void  Timer::_SetCallback(void *timer,const void *callback){
        static_cast<TimerImpl*>(timer)->set_callback(
            static_cast<const _TimerInvokerData*>(callback)
        );
    }

    // Uint32 TimerImpl::invoke(Uint32 new_interval){
    //     interval = new_interval;
    //     //Lock data begin call the invoker
    //     lock_guard<SpinLock> locker(data_spinlock);
    //     if(data.invoke == nullptr){
    //         return interval;
    //     }
    //     else{
    //         try{
    //             interval = data.invoke(interval,data.invoker);
    //         }
    //         catch(...){
    //             DeferCall(std::rethrow_exception,std::current_exception());
    //         }
    //     }
    //     Uint32 i = interval.load(std::memory_order::memory_order_acquire);
    //     if(i == 0){
    //         running = false;
    //         timer_id = 0;
    //     }
    //     return i;
    // }
    // void TimerImpl::cleanup_invoker(){
    //     lock_guard<SpinLock> locker(data_spinlock);

    //     if(data.invoker != nullptr){
    //         data.cleanup(data.invoker);

    //         data.invoker = nullptr;
    //         data.cleanup = nullptr;
    //         data.invoke = nullptr;
    //     }
    // }
    // void Timer::bind(_TimerInvokerData data){
    //     timer->cleanup_invoker();
    //     lock_guard<SpinLock> locker(timer->data_spinlock);
    //     timer->data = data;
    // }
    // void Timer::stop(){
    //     lock_guard<SpinLock> locker(timer->data_spinlock);
    //     if(timer->running){
    //         bool v = SDL_RemoveTimer(timer->timer_id);
    //         timer->running = false;
    //         timer->timer_id = 0;
    //     }
    // }
    // void Timer::start(){
    //     lock_guard<SpinLock> locker(timer->data_spinlock);
    //     if(not timer->running){
    //         timer->running = true;
    //         timer->timer_id = SDL_AddTimer(
    //             timer->interval,
    //             TimerImpl::Entry,
    //             timer
    //         );
    //     }
    // }
    // void Timer::set_interval(Uint32 interval){
    //     timer->interval = interval;
    // }
    // Uint32 Timer::interval() const{
    //     return timer->interval;
    // }
    // bool Timer::running() const{
    //     return timer->running;
    // }
    // Timer::Timer(){
    //     timer = new TimerImpl;
    // }
    // Timer::~Timer(){
    //     stop();
    //     delete timer;
    // }
}