#if !defined(_BTK_IMPL_THREAD_HPP_)
#define _BTK_IMPL_THREAD_HPP_
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_mutex.h>
#include <memory>
#include <tuple>

#include "../exception.hpp"
#include "../utils/sync.hpp"

namespace Btk{
    //SDL_Thread
    template<class T,class ...Args>
    struct _ThreadInvoker:public std::tuple<Args...>{
        T callable;
        
        //A Simple invoker for SDL_Thread
        static int SDLCALL Run(void *__self){
            //It is easy to move
            if constexpr(std::is_nothrow_move_assignable_v<_ThreadInvoker>
                        and std::is_trivially_move_assignable_v<_ThreadInvoker>
                ){
                _ThreadInvoker self = std::move(*static_cast<_ThreadInvoker*>(__self));
                delete static_cast<_ThreadInvoker*>(__self);
                std::apply(self.callable,static_cast<std::tuple<Args...>&&>(self));
            }
            else{
                std::unique_ptr<_ThreadInvoker> ptr(static_cast<_ThreadInvoker*>(__self));
                std::apply(ptr->callable,static_cast<std::tuple<Args...>&&>(*ptr));
            }
            return 0;
        }
    };
    /**
     * @brief ThreadPriority enum for thread priority
     * 
     */
    enum class ThreadPriority:int{
        Idle = SDL_THREAD_PRIORITY_LOW,
        Low = SDL_THREAD_PRIORITY_LOW,
        Normal = SDL_THREAD_PRIORITY_NORMAL,
        High = SDL_THREAD_PRIORITY_HIGH,
        Realtime = SDL_THREAD_PRIORITY_TIME_CRITICAL
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
                using InvokerType = _ThreadInvoker
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
                using InvokerType = _ThreadInvoker
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
            const char *name(){
                return SDL_GetThreadName(thrd);
            };
            
            Thread &operator =(Thread &&th){
                thrd = th.thrd;
                th.thrd = nullptr;
                return *this;
            };
        private:
            SDL_Thread *thrd;
    };
    /**
     * @brief Set the Thread Priority object on the current thread
     * 
     * @param priority The thread priority
     * @param errcode The error code reference
     */
    inline void SetThreadPriority(ThreadPriority priority,int &errcode) noexcept{
        errcode = SDL_SetThreadPriority(static_cast<SDL_ThreadPriority>(priority));
    };
    /**
     * @brief Set the Thread Priority object on the current thread
     * 
     * @param priority The thread priority
     */
    inline void SetThreadPriority(ThreadPriority priority){
        if(SDL_SetThreadPriority(static_cast<SDL_ThreadPriority>(priority)) != 0){
            throwSDLError();
        }
    };
}


#endif // _BTK_IMPL_THREAD_HPP_
