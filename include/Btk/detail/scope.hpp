#if !defined(_BTKIMPL_SCOPE_HPP_)
#define _BTKIMPL_SCOPE_HPP_
#include <SDL2/SDL_stdinc.h>
#include <cstdarg>
#include "../defs.hpp"

#define Btk_defer Btk::Impl::ScopeGuard BTK_UNIQUE_NAME(_defer__) = 

#ifdef __GNUC__
    #define Btk_CallOnLoad static __attribute__((constructor)) void BTK_UNIQUE_NAME(_onload_executer__)()
    #define Btk_CallOnUnload static __attribute__((destructor)) void BTK_UNIQUE_NAME(_onunload_executer__)()
#else
    #define Btk_CallOnLoad static Btk::Impl::OnloadExecuter BTK_UNIQUE_NAME(_onload_executer__) = []()
    #define Btk_CallOnUnload static Btk::Impl::OnUnloadExecuter BTK_UNIQUE_NAME(_onunload_executer__) = []()
#endif

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
        ~SDLScopePtr(){
            SDL_free(data);
        }
        void *data;
    };
    /**
     * @brief A helper class for use va_list with RAII
     * 
     */
    struct VaListGuard{
        inline VaListGuard(va_list &v):varg(v){}
        inline VaListGuard(const VaListGuard &) = delete;
        inline ~VaListGuard(){
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

    template<class T>
    struct OnloadExecuter{
        OnloadExecuter(T &&v){
            v();
        }
    };

    template<class T>
    struct OnUnloadExecuter{
        T fn;
        OnUnloadExecuter(T &&v):fn(v){

        }
        ~OnUnloadExecuter(){
            fn();
        }
    };
};
    using Impl::SDLScopePtr;
};

#endif // _BTKIMPL_SCOPE_HPP_
