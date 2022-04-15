#if !defined(_BTK_EVENT_LOOP_HPP_)
#define _BTK_EVENT_LOOP_HPP_
#include "utils/template.hpp"
#include "defs.hpp"
//Minimal event loop interface

namespace Btk{
    /**
     * @brief Loop status
     * 
     */
    class LoopStatus{
        public:
            static constexpr Uint8 Running = 1;//< Loop is running
            static constexpr Uint8 Interrupted = 2;//< Interrupted by user
            static constexpr Uint8 Exception = 3;//< Exception occured
        public:
            LoopStatus(Uint8 _v):v(_v){}
            LoopStatus() = default;
            LoopStatus(const LoopStatus &) = default;
            ~LoopStatus() = default;


            LoopStatus &operator =(const LoopStatus &) = default;

            operator bool() const noexcept{
                return v == Running;
            }
            operator Uint8() const noexcept{
                return v;
            }
        private:
            Uint8 v = 0;
    };
    /**
     * @brief This function will be called in main EventLoop
     * 
     * @param fn The function you want to call
     * @param data User data
     */
    BTKAPI void DeferCall(void(* fn)(void*),void *data);
    BTKAPI void DeferCall(void(* fn)());
    /**
     * @brief Push a function to call in Asyncronous way
     * 
     * @param fn 
     * @param data 
     * @return BTKAPI 
     */
    BTKAPI void AsyncCall(void (*fn)(void*),void *data);
    /**
     * @brief Register a callback which will be called in idle
     * 
     * @param fn 
     * @param data 
     * @return BTKAPI 
     */
    BTKAPI void AtIdle(void(* fn)(void*),void *data);
    BTKAPI void AtIdle(void(* fn)());
    /**
     * @brief Regitser atexit callback
     * 
     * @param fn Callback function
     * @param data User data
     */
    BTKAPI void AtExit(void(* fn)(void*),void *data);
    BTKAPI void AtExit(void(* fn)());
    /**
     * @brief Push current exception to the event loop
     * 
     * @return BTKAPI 
     */
    BTKAPI void DeferRethrow();
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
    BTKAPI auto PollEvent() -> LoopStatus;
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
    BTKAPI auto WaitEvent() -> LoopStatus;
    /**
     * @brief Interrupt the event loop
     * 
     * @return BTKAPI 
     */
    BTKAPI void InterruptLoop();

    /**
     * @brief End the event loop
     * 
     * @param code The exit code
     */
    BTKAPI void Exit(int code = EXIT_SUCCESS);
    /**
     * @brief Enter the EventLoop
     * 
     * @return 0 if succeed
     */
    BTKAPI int  run();
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
}

namespace Btk{
    /**
     * @brief Register a callback to be called in exit
     * 
     * @tparam Callable The callback type
     * @tparam Args The args type 
     * @param callable The callback
     * @param args The args
     */
    template<class Callable,class ...Args>
    void AtExit(Callable &&callable,Args ...args){
        auto *invoker = new _OnceInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = _OnceInvoker<Callable,Args...>::Run;
        AtExit(fn,data);
    }
    /**
     * @brief Register a callback to be called in idle
     * 
     * @tparam Callable 
     * @tparam Args 
     * @param callable 
     * @param args 
     */
    template<class Callable,class ...Args>
    void AtIdle(Callable &&callable,Args ...args){
        auto *invoker = new _GenericInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = _GenericInvoker<Callable,Args...>::Run;
        AtIdle(fn,data);
        //We need to destroy the object
        void(* cleanup)(void*) = [](void *invoker){
            delete static_cast<_GenericInvoker<Callable,Args...>*>(invoker);
        };
        AtExit(cleanup,data);
    }
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
        auto *invoker = new _OnceInvoker<Callable,Args...>{
            {std::forward<Args>(args)...},
            std::forward<Callable>(callable)
        };
        void *data = invoker;
        void (*fn)(void*) = _OnceInvoker<Callable,Args...>::Run;
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
}

#endif // _BTK_EVENT_LOOP_HPP_
