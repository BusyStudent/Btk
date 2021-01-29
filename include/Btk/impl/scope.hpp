#if !defined(_BTKIMPL_SCOPE_HPP_)
#define _BTKIMPL_SCOPE_HPP_
#include <SDL2/SDL_stdinc.h>
#include <cstdarg>
#define Btk_defer Btk::Impl::ScopeGuard __GUARD__ = [&]()
namespace Btk{
namespace Impl{
    //ScopeGuard
    template<class T>
    struct ScopeGuard{
        inline ScopeGuard(T &&f):fn(f){}
        inline ~ScopeGuard(){
            fn();
        }
        T &fn;
    };
    //Data from SDL lib
    struct SDLScopePtr{
        SDLScopePtr(void *dat):data(dat){};
        SDLScopePtr(const SDLScopePtr & ) = delete;
        SDLScopePtr(){
            SDL_free(data);
        }
        void *data;
    };
    /**
     * @brief A helper class for use va_list with RAII
     * 
     */
    struct VaListGuard{
        VaListGuard(va_list &v):varg(v){}
        VaListGuard(const VaListGuard &) = delete;
        ~VaListGuard(){
            va_end(varg);
        }
        va_list &varg;
    };

    template<class T>
    struct ScopePtr{
        ScopePtr(T *p):ptr(p){}
        ScopePtr(const ScopePtr &) = delete;
        ~ScopePtr(){
            if(owned){
                delete ptr;
            }
        }
        void release(){
            owned = false;
        }
        T *operator &() const noexcept{
            return ptr;
        }
        T *operator ->() const noexcept{
            return ptr;
        }
        T *get() const noexcept{
            return ptr;
        }
        T *ptr;
        bool  owned = true;
    };
};
    using Impl::SDLScopePtr;
};

#endif // _BTKIMPL_SCOPE_HPP_
