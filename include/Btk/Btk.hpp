#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include <type_traits>
#include <exception>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <tuple>
#include "defs.hpp"
#include "detail/invoker.hpp"
#include "utils/traits.hpp"
#include "window.hpp"
namespace Btk{

    typedef bool(*ExceptionHandler)(std::exception*);
    BTKAPI ExceptionHandler SetExceptionHandler(ExceptionHandler);
    /**
     * @brief End the event loop
     * 
     * @param code The exit code
     */
    BTKAPI void Exit(int code = EXIT_SUCCESS);
    BTKAPI void Init();
    /**
     * @brief Regitser atexit callback
     * 
     * @param fn Callback function
     * @param data User data
     */
    BTKAPI void AtExit(void(* fn)(void*),void *data);
    BTKAPI void AtExit(void(* fn)());

    template<class Callable,class ...Args>
    void AtExit(Callable &&callable,Args ...args){
        auto *invoker = new Impl::OnceInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::OnceInvoker<Callable,Args...>::Run;
        AtExit(fn,data);
    }

    /**
     * @brief Register a callback which will be called in idle
     * 
     * @param fn 
     * @param data 
     * @return BTKAPI 
     */
    BTKAPI void AtIdle(void(* fn)(void*),void *data);
    BTKAPI void AtIdle(void(* fn)());

    BTKAPI void AsyncCall(void (*fn)(void*),void *data);
    

    template<class Callable,class ...Args>
    void AtIdle(Callable &&callable,Args ...args){
        auto *invoker = new Impl::GenericInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::GenericInvoker<Callable,Args...>::Run;
        AtIdle(fn,data);
        //We need to destroy the object
        void(* cleanup)(void*) = [](void *invoker){
            delete static_cast<Impl::GenericInvoker<Callable,Args...>*>(invoker);
        };
        AtExit(cleanup,data);
    }

    /**
     * @brief This function will be called in main EventLoop
     * 
     * @param fn The function you want to call
     * @param data User data
     */
    BTKAPI void DeferCall(void(* fn)(void*),void *data);
    BTKAPI void DeferCall(void(* fn)());
    /**
     * @brief This function will be called in main EventLoop
     * 
     * @tparam Callable 
     * @tparam Args 
     * @param callable 
     * @param args 
     */
    template<
        class Callable,
        class ...Args,
        typename _Cond = std::enable_if_t<!std::is_member_function_pointer_v<Callable>>
    >
    void DeferCall(Callable &&callable,Args ...args){
        auto *invoker = new Impl::OnceInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::OnceInvoker<Callable,Args...>::Run;
        DeferCall(fn,data);
    }
    /**
     * @brief Defercall for Object member function
     * 
     * @tparam Callable 
     * @tparam Object 
     * @tparam Args 
     * @tparam _Cond 
     * @param callable 
     * @param object 
     * @param args 
     */
    template<
        class Callable,
        class Object,
        class ...Args,
        typename _Cond = std::enable_if_t<std::is_member_function_pointer_v<Callable>>
    >
    void DeferCall(Callable &&callable,Object *object,Args ...args){
        static_cast<HasSlots*>(object)->defer_call(
            std::forward<Callable>(callable),
            std::forward<Args>(args)...
        );
    }
    BTKAPI void DeferRethrow();
    /**
     * @brief Check is main thread(which call Btk::run)
     * 
     * @return true,if is the main thread
     */
    BTKAPI bool IsMainThread();
    /**
     * @brief Check is not the main thread 
     *        or Main EventLoop is not running
     * 
     * @return true on we can do sth blcok the thread 
     */
    BTKAPI bool CouldBlock();
    /**
     * @brief Hide the console
     * @note You would be better to use it in main thread
     * 
     * @return true on succeess
     */
    BTKAPI bool HideConsole();
    /**
     * @brief Poll event and update status 
     * 
     * @code {.cpp}
     * while(PollEvent()){
     *     //Process your ...
     * }
     * @endcode
     * 
     * 
     * @return true on continue
     * @return false on asked to quit
     */
    BTKAPI bool PollEvent();
    /**
     * @brief Wait event and update status
     * 
     * @code {.cpp}
     * while(WaitEvent()){
     *     //Process your ...
     * }
     * @endcode
     * 
     * 
     * @return BTKAPI 
     */
    BTKAPI bool WaitEvent();

    /**
     * @brief Enter the EventLoop
     * 
     * @return 0 if succeed
     */
    BTKAPI int  run();
}

#endif // _BTK_HPP_
