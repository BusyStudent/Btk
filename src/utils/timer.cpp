#include "../build.hpp"

#include <Btk/impl/thread.hpp>
#include <Btk/impl/atomic.hpp>
#include <Btk/utils/timer.hpp>
#include <Btk/exception.hpp>
#include <Btk/Btk.hpp>

#include <SDL2/SDL_timer.h>
#include <exception>
#include <mutex>

namespace Btk{
    struct TimerInvoker{
        void (*entry)(void*);
        void (*delete_self)(void*);
        void *invoker;
        void operator ()() const{
            if(not empty()){
                return entry(invoker);
            }
        }
        bool empty() const {
            return entry == nullptr;
        }
        void cleanup() noexcept{
            if(not empty()){
                delete_self(invoker);
                entry = nullptr;
                delete_self = nullptr;
                invoker = nullptr;
            }
        }
    };
}
namespace Btk{
    /**
     * @brief The Timer Impl
     * 
     */
    struct TimerBase{
        TimerBase(){
            SDL_zerop(this);
        }
        ~TimerBase();
        Uint32 run();

        SDL_TimerID timerid;
        
        Uint32  interval;//Timer's time out
        SpinLock mtx;//THe data mutex
        TimerInvoker invoker;//Timer invoker

        Atomic running;//Is timer running
    };
    //Timer's entry
    Uint32 SDLCALL TimerRun(Uint32,void *timerbase){
        BTK_ASSERT(timerbase != nullptr);
        auto *timer = static_cast<TimerBase*>(timerbase);
        Uint32 ret = timer->run();
        if(ret == 0){
            //Is not running
            timer->running = false;
        }
        return ret;
    }


    TimerBase::~TimerBase(){
        if(timerid != 0){
            BTK_ASSERT(SDL_RemoveTimer(timerid));
        }
        //Cleanup the invoker
        invoker.cleanup();
    }
    //Run the timer
    Uint32 TimerBase::run(){
        //Get it's invoker
        mtx.lock();
        auto cb = invoker;
        mtx.unlock();

        //Rethrow the exception at main thread
        try{
            cb();
        }
        catch(...){
            Btk::DeferCall(std::rethrow_exception,std::current_exception());
        }

        std::lock_guard locker(mtx);
        return interval;
    }
}
namespace Btk{
    Timer::Timer(){
        base = new TimerBase();
    }
    Timer::~Timer(){
        delete base;
    }
    Timer& Timer::set_interval(Uint32 interval){
        std::lock_guard locker(base->mtx);
        base->interval = interval;
        return *this;
    }
    Timer& Timer::start(){
        stop();
        
        base->timerid = SDL_AddTimer(
            base->interval,
            TimerRun,
            base
        );
        if(base->timerid == 0){
            throwSDLError();
        }
        return *this;
    }
    Timer& Timer::stop(){
        if(base->running){
            SDL_RemoveTimer(base->timerid);
            base->timerid = 0;
            base->running = false;
        }
        return *this;
    }
    void Timer::set_invoker(
        InvokerRunFn run_fn,
        InvokerCleanupFn cleanup_fn,
        void *invoker){

        std::lock_guard locker(base->mtx);
        //remove the old one
        base->invoker.cleanup();

        base->invoker.invoker = invoker;
        base->invoker.entry = run_fn;
        base->invoker.delete_self = cleanup_fn;
    }
    bool Timer::running() const{
        return base->running;
    }
    Uint32 Timer::interval() const{
        std::lock_guard locker(base->mtx);
        return base->interval;
    }
}