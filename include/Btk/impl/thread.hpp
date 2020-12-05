#if !defined(_BTK_IMPL_THREAD_HPP_)
#define _BTK_IMPL_THREAD_HPP_
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <memory>
#include <tuple>

#include "../exception.hpp"
namespace Btk{
    //SDL_Thread
    namespace Impl{
        template<class T,class ...Args>
        struct ThreadInvoker{
            T callable;
            std::tuple<Args...> args;
            //A Simple invoker for SDL_Thread
            static int SDLCALL Run(void *__self){
                std::unique_ptr<ThreadInvoker> ptr(static_cast<ThreadInvoker*>(__self));
                std::apply(ptr->callable,ptr->args);
                return 0;
            }
        };
    };
    class Thread{
        public:
            /**
             * @brief Construct a new Thread object
             * 
             * @tparam Callable The Callable object type
             * @tparam Args The Args type for the objects
             * @param callable The thread entry
             * @param args The thread args
             */
            template<class Callable,class ...Args>
            Thread(Callable &&callable,Args ...args){
                using InvokerType = Impl::ThreadInvoker
                    <std::remove_reference_t<Callable>,Args...>;
                
                
                auto *invoker = new InvokerType{
                    std::forward<Callable>(callable),
                    {std::forward<Args>(args)...}
                };
                thrd = SDL_CreateThread(
                    InvokerType::Run,
                    nullptr,
                    invoker
                );
            };
            Thread():thrd(nullptr){};
            Thread(const Thread &) = delete;
            Thread(Thread &&t):thrd(t.thrd){
                t.thrd = nullptr;
            };
            ~Thread(){
                //Default to wait thread
                SDL_WaitThread(thrd,nullptr);
            };


            //methods
            void wait(){
                SDL_WaitThread(thrd,nullptr);
                thrd = nullptr;
            };
            void join(){
                wait();
            };
            void detach(){
                SDL_DetachThread(thrd);
                thrd = nullptr;
            };
            
            Thread &operator =(Thread &&th){
                thrd = th.thrd;
                th.thrd = nullptr;
                return *this;
            };
        private:
            SDL_Thread *thrd;
    };
    class SpinLock{
        public:
            void lock() noexcept{
                SDL_AtomicLock(&slock);
            };
            void unlock() noexcept{
                SDL_AtomicUnlock(&slock);
            };
            bool try_lock() noexcept{
                return SDL_AtomicTryLock(&slock);
            };
        private:
            SDL_SpinLock slock = 0;
    };
};


#endif // _BTK_IMPL_THREAD_HPP_
