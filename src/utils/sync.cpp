#include "../build.hpp"

#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_mutex.h>
#include <Btk/utils/sync.hpp>
#include <Btk/exception.hpp>

#include <mutex>

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
        return true;
    }
}