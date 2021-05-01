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
        struct ThreadInvoker:public std::tuple<Args...>{
            T callable;
            
            //A Simple invoker for SDL_Thread
            static int SDLCALL Run(void *__self){
                //It is easy to move
                if constexpr(std::is_nothrow_move_assignable<ThreadInvoker>() 
                         and std::is_trivially_move_assignable<ThreadInvoker>()
                    ){
                    ThreadInvoker self = std::move(*static_cast<ThreadInvoker*>(__self));
                    delete static_cast<ThreadInvoker*>(__self);
                    std::apply(self.callable,static_cast<std::tuple<Args...>&&>(self));
                }
                else{
                    std::unique_ptr<ThreadInvoker> ptr(static_cast<ThreadInvoker*>(__self));
                    std::apply(ptr->callable,static_cast<std::tuple<Args...>&&>(*ptr));
                }
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
                    {std::forward<Args>(args)...},
                    std::forward<Callable>(callable)
                };
                thrd = SDL_CreateThread(
                    InvokerType::Run,
                    nullptr,
                    invoker
                );
            };
            template<class Callable,class ...Args>
            Thread(const char *name,Callable &&callable,Args ...args){
                using InvokerType = Impl::ThreadInvoker
                    <std::remove_reference_t<Callable>,Args...>;
                
                
                auto *invoker = new InvokerType{
                    {std::forward<Args>(args)...},
                    std::forward<Callable>(callable)
                };
                thrd = SDL_CreateThread(
                    InvokerType::Run,
                    name,
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
    class Semaphore{
        public:
            /**
             * @brief Construct a new Semaphore object
             * 
             * @param value The initial value(default to 0)
             */
            Semaphore(Uint32 value = 0){
                sem = SDL_CreateSemaphore(value);
                if(sem == nullptr){
                    throwSDLError();
                }
            }
            Semaphore(const Semaphore &) = delete;
            Semaphore(Semaphore &&s){
                sem = s.sem;
                s.sem = nullptr;
            }
            ~Semaphore(){
                SDL_DestroySemaphore(sem);
            }
            /**
             * @brief Get current value
             * 
             * @return Uint32 
             */
            Uint32 value() const{
                return SDL_SemValue(sem);
            }
            
        private:
            SDL_sem *sem;
    };
    using Sem = Semaphore;
};


#endif // _BTK_IMPL_THREAD_HPP_
