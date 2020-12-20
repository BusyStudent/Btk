#if !defined(_BTKUTILS_TIMER_HPP_)
#define _BTKUTILS_TIMER_HPP_
#include <memory>
#include <tuple>
#include "../defs.hpp"
namespace Btk{
    namespace Impl{
        template<class Callable,class ...Args>
        struct TimerInvoker:public std::tuple<Args...>{
            Callable callable;
            static void Run(void *__self){
                auto *self = static_cast<TimerInvoker*>(__self);
                std::apply(self->callable,std::forward<std::tuple<Args...>&&>(*self));
            }
            static void Delete(void *self){
                delete static_cast<TimerInvoker*>(self);
            }
        };
    }
    struct TimerBase;
    class BTKAPI Timer{
        public:
            Timer();
            Timer(const Timer &) = delete;
            Timer(Timer &&t){
                base = t.base;
                t.base = nullptr;
            };
            ~Timer();
            Timer& set_interval(Uint32 interval);
            Timer& start();
            Timer& stop();

            template<class Callable,class ...Args>
            Timer& set_callback(Callable &&callable,Args &&...args){
                using Invoker = Impl::TimerInvoker<Callable,Args...>;
                void *invoker = new Invoker{
                    {std::forward<Args>(args)...},
                    std::forward<Callable>(callable)
                };
                set_invoker(
                    Invoker::Run,
                    Invoker::Delete,
                    invoker
                );
                return *this;
            }
            /**
             * @brief Check the timer is running
             * 
             * @return true 
             * @return false 
             */
            bool   running() const;
            /**
             * @brief Get the timer's interval
             * 
             * @return The interval
             */
            Uint32 interval()const;
        private:
            typedef void (*InvokerRunFn)(void*);
            typedef void (*InvokerCleanupFn)(void*);
            /**
             * @brief Internal method for set invoker
             * 
             */
            void set_invoker(InvokerRunFn,InvokerCleanupFn,void*);
            TimerBase *base;
    };
}


#endif // _BTKUTILS_TIMER_HPP_
