#include "../build.hpp"

#include <SDL2/SDL_atomic.h>
#include <SDL2/SDL_mutex.h>
#include <Btk/utils/sync.hpp>
#include <Btk/exception.hpp>

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

}