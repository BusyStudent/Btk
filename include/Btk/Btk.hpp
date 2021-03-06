#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include <type_traits>
#include <exception>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <tuple>
#include "defs.hpp"
#include "utils/traits.hpp"
#include "impl/invoker.hpp"
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
        auto *invoker = new Impl::Invoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = Impl::Invoker<Callable,Args...>::Run;
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
     * @brief Enter the EventLoop
     * 
     * @return 0 if succeed
     */
    BTKAPI int  run();
}

#endif // _BTK_HPP_
