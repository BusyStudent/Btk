#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include <exception>
#include <cstdint>
#include "defs.hpp"
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
    /**
     * @brief This function will be called in main EventLoop
     * 
     * @param fn The function you want to call
     * @param data User data
     */
    extern void DeferCall(void(* fn)(void*),void *data);
    extern void DeferCall(void(* fn)());
    /**
     * @brief Enter the EventLoop
     * 
     * @return 0 if succeed
     */
    extern int  run();
};

#endif // _BTK_HPP_
