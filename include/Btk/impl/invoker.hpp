#if !defined(_BTKIMPL_INVOKER_HPP_)
#define _BTKIMPL_INVOKER_HPP_
//Impl for Impl::Invoker
#include <tuple>
#include <memory>
namespace Btk{
    namespace Impl{
        template<class Callable,class ...Args>
        struct Invoker:public std::tuple<Args...>{
            Callable callable;
            static void Run(void *__self){
                Invoker *self = static_cast<Invoker*>(__self);
                std::unique_ptr<Invoker> ptr(self);
                std::apply(std::forward<Callable>(ptr->callable),
                           std::forward<std::tuple<Args...>&&>(*ptr));
                
            }
        };
    }
}
#endif // _BTKIMPL_INVOKER_HPP_
