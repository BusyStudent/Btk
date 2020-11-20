#if !defined(BTK_ASYNC_HPP_)
#define BTK_ASYNC_HPP_
#include <tuple>
#include "../signal/signal.hpp"
#include "../defs.hpp"
namespace Btk{
    namespace Impl{
        template<class RetT>
        struct AsyncResult{
            Signal<void(RetT)> sig;
        };
        template<>
        struct AsyncResult<void>{
            Signal<void()> sig;
        };
        template<class Callable,class ...Args>
        struct AsyncPackage{
            AsyncPackage(Callable &&c,Args &&...args):
                callable(std::forward<Callable>(c)),
                args_pak(std::forward<Args>(args)...){
                
            }
            Callable callable;
            std::tuple<Args...> args_pak;
            //AsyncResult
            AsyncResult<std::invoke_result_t<Callable,Args...>> aret;
            static void Run(void *__self){
                auto *self = static_cast<AsyncPackage*>(__self);
                if constexpr(std::is_same<void,std::invoke_result_t<Callable,Args...>>::value){
                    std::apply(self->callable,self->args_pak);
                    auto &sig = self->aret.sig;
                    if(not sig.empty()){
                        sig.emit();
                    }
                }
                else{
                    auto ret = std::apply(self->callable,self->args_pak);
                    auto &sig = self->aret.sig;
                    if(not sig.empty()){
                        sig.emit(ret);
                    }
                }
                delete self;
            };
        };
        /**
         * @brief Lunch a AsyncPackage right now
         * 
         * @param package the Package pointer
         * @param run the Package entry
         */
        BTKAPI void RtLunch(void *package,void(*run)(void*package));
        BTKAPI void DeferLunch(void *package,void(*run)(void*package));
    };
    template<class Callable,class ...Args>
    struct AsyncTask{
        Impl::AsyncPackage<Callable,Args...> *package;
        void lunch();
        //lunch it right now
        void rt_lunch(){
            Impl::RtLunch(package,Impl::AsyncPackage<Callable,Args...>::Run);
        };
    };
    //Create a task
    template<class Callable,class ...Args>
    AsyncTask<Callable,Args...> Async(Callable &&callable,Args &&...args){
        AsyncTask<Callable,Args...> task;
        task.package = new Impl::AsyncPackage<Callable,Args...>(
            std::forward<Callable>(callable),
            std::forward<Args>(args)...
        );
        return task;
    };
};


#endif // BTK_ASYNC_HPP_
