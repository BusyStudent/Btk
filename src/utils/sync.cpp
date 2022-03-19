#include "../build.hpp"

#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_mutex.h>
#include <Btk/utils/sync.hpp>
#include <Btk/exception.hpp>

#include <mutex>

#if BTK_GCC && (defined(__i386__) || defined(__x86_64__))
    #define PAUSE() __asm__ __volatile__("pause\n")
#else
    #define PAUSE()
#endif


namespace Btk{
    void SpinLock::lock() noexcept{
        SDL_AtomicLock(&slock);
    }
    void SpinLock::unlock() noexcept{
        SDL_AtomicUnlock(&slock);
    }
    bool SpinLock::try_lock() noexcept{
        return SDL_AtomicTryLock(&slock);
    }
    //RSpinlock
    void RSpinLock::lock() noexcept{
        int iterations = 0;
        while(not try_lock()){
            //Sleep here
            if(iterations < 32){
                iterations ++;
                PAUSE();
                continue;
            }
            SDL_Delay(0);
        }
    }
    void RSpinLock::unlock() noexcept{
        BTK_ASSERT(owner == SDL_ThreadID());
        BTK_ASSERT(locked);
        counts --;
        if(counts == 0){
            locked.store(false,std::memory_order_release);
            owner = 0;
        }
    }
    bool RSpinLock::try_lock() noexcept{
        if(locked.load(std::memory_order_acquire)){
            if(owner == SDL_ThreadID()){
                counts ++;
            }
            else{
                //No locked
                return false;
            }
            return true;
        }
        //Try to get it
        locked.store(true,std::memory_order_release);
        owner = SDL_ThreadID();
        counts = 1;
        return true;
    }
    /**
     * @brief Construct a new Semaphore object
     * 
     * @param value The initial value(default to 0)
     */
    Semaphore::Semaphore(Uint32 value){
        sem = SDL_CreateSemaphore(value);
        if(sem == nullptr){
            throwSDLError();
        }
    }
    Semaphore::~Semaphore(){
        SDL_DestroySemaphore(sem);
    }
    /**
     * @brief Get current value
     * 
     * @return Uint32 
     */
    Uint32 Semaphore::value() const{
        return SDL_SemValue(sem);
    }
    void Semaphore::post(){
        if(SDL_SemPost(sem) != 0){
            throwSDLError();
        }
    }
    void Semaphore::wait(){
        if(SDL_SemWait(sem) != 0){
            throwSDLError();
        }
    }
    //Event
    SyncEvent::SyncEvent(){
        cond = SDL_CreateCond();
        mtx  = SDL_CreateMutex();
        if(cond == nullptr or mtx == nullptr){
            u8string err = SDL_GetError();
            SDL_DestroyCond(cond);
            SDL_DestroyMutex(mtx);
            throwSDLError(err);
        }
    }
    SyncEvent::~SyncEvent(){
        SDL_DestroyCond(cond);
        SDL_DestroyMutex(mtx);
    }
    bool SyncEvent::is_set() const noexcept{
        std::lock_guard locker(it_lock);
        return isset;
    }
    void SyncEvent::set(){
        std::lock_guard locker(it_lock);
        if(isset){
            //No need to boardcast
            return;
        }
        isset = true;
        SDL_CondBroadcast(cond);
    }
    void SyncEvent::clear(){
        std::lock_guard locker(it_lock);
        isset = false;
    }
    //Wait
    void SyncEvent::wait(){
        SDL_LockMutex(mtx);
        while(not is_set()){
            SDL_CondWait(cond,mtx);
        }
        SDL_UnlockMutex(mtx);
    }
    bool SyncEvent::wait(Uint32 ms){
        bool val = true;
        SDL_LockMutex(mtx);
        while(not is_set()){
            if(SDL_CondWaitTimeout(cond,mtx,ms) == SDL_MUTEX_TIMEDOUT){
                //Timeout
                val = false;
                break;
            }
        }
        SDL_UnlockMutex(mtx);
        return val;
    }
}