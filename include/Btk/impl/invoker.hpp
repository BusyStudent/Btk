#if !defined(_BTKIMPL_INVOKER_HPP_)
#define _BTKIMPL_INVOKER_HPP_
//Impl for Impl::Invoker
#include <tuple>
#include <memory>
namespace Btk{
    namespace Impl{
        /**
         * @brief Call and delete self
         * 
         * @tparam Callable 
         * @tparam Args 
         */
        template<class Callable,class ...Args>
        struct OnceInvoker:public std::tuple<Args...>{
            Callable callable;
            static void Run(void *__self){
                OnceInvoker *self = static_cast<OnceInvoker*>(__self);
                std::unique_ptr<OnceInvoker> ptr(self);
                std::apply(std::forward<Callable>(ptr->callable),
                           std::forward<std::tuple<Args...>&&>(*ptr));
                
            }
        };
        template<class Callable,class ...Args>
        struct GenericInvoker:public std::tuple<Args...>{
            Callable callable;
            static void Run(void *__self){
                GenericInvoker *ptr = static_cast<GenericInvoker*>(__self);
                std::apply(std::forward<Callable>(ptr->callable),
                           std::forward<std::tuple<Args...>&&>(*ptr));
                
            }
        };
    }
}
#endif // _BTKIMPL_INVOKER_HPP_
