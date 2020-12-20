#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include <exception>
#include <cstdint>
#include <memory>
#include <tuple>
#include "impl/invoker.hpp"
#include "window.hpp"
namespace Btk{

    typedef bool(*ExceptionHandler)(std::exception*);
    extern ExceptionHandler SetExceptionHandler(ExceptionHandler);
    extern void Init();
    /**
     * @brief Regitser atexit callback
     * 
     * @param fn Callback function
     * @param data User data
     */
    extern void AtExit(void(* fn)(void*),void *data);
    extern void AtExit(void(* fn)());

    template<class Callable,class ...Args>
    void AtExit(Callable &&callable,Args &&...args){
        auto *invoker = new Impl::Invoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::Invoker<Callable,Args...>::Run;
        AtExit(fn,data);
    }
    /**
     * @brief This function will be called in main EventLoop
     * 
     * @param fn The function you want to call
     * @param data User data
     */
    extern void DeferCall(void(* fn)(void*),void *data);
    extern void DeferCall(void(* fn)());
    
    template<class Callable,class ...Args>
    void DeferCall(Callable &&callable,Args &&...args){
        auto *invoker = new Impl::Invoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::Invoker<Callable,Args...>::Run;
        DeferCall(fn,data);
    }
    /**
     * @brief Enter the EventLoop
     * 
     * @return 0 if succeed
     */
    extern int  run();
};

#endif // _BTK_HPP_
