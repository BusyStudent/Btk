#if !defined(_BTKIMPL_ATOMIC_HPP_)
#define _BTKIMPL_ATOMIC_HPP_
#include <SDL2/SDL_atomic.h>
namespace Btk{
    //a wrapper for SDL_atomic_t
    struct Atomic{
        Atomic(){}
        //uninited value
        Atomic(int val){
            SDL_AtomicSet(&value,val);
        }
        mutable SDL_atomic_t value;
        Atomic &operator =(int val){
            SDL_AtomicSet(&value,val);
            return *this;
        }
        Atomic &operator ++(){
            SDL_AtomicIncRef(&value);
            return *this;
        }
        Atomic &operator --(){
            SDL_AtomicDecRef(&value);
            return *this;
        }
        operator int() const noexcept{
            return SDL_AtomicGet(&value);
        }
        //like normal int
        Atomic operator +(int value) const noexcept{
            return Atomic(static_cast<int>(*this) + value);
        }
        Atomic operator -(int value) const noexcept{
            return Atomic(static_cast<int>(*this) - value);
        }
        Atomic operator *(int value) const noexcept{
            return Atomic(static_cast<int>(*this) * value);
        }
        Atomic operator /(int value) const noexcept{
            return Atomic(static_cast<int>(*this) / value);
        }
    };
};

#endif // _BTKIMPL_ATOMIC_HPP_
