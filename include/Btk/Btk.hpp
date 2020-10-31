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
    //Regitser atexit callback
    extern void AtExit(void(*)(void*),void *data);
    extern void AtExit(void(*)());
    extern int  run();
};

#endif // _BTK_HPP_
