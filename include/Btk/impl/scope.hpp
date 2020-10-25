#if !defined(_BTKIMPL_SCOPE_HPP_)
#define _BTKIMPL_SCOPE_HPP_
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
};
};

#endif // _BTKIMPL_SCOPE_HPP_
