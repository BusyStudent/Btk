#if !defined(_BTKIMPL_SCOPE_HPP_)
#define _BTKIMPL_SCOPE_HPP_
#include <SDL2/SDL_stdinc.h>
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
};
    using Impl::SDLScopePtr;
};

#endif // _BTKIMPL_SCOPE_HPP_
