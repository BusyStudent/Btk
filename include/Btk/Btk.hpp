#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include <exception>
#include "defs.hpp"
#include "window.hpp"
namespace Btk{
    typedef bool(*ExceptionHandler)(std::exception*);
    extern ExceptionHandler SetExceptionHandler(ExceptionHandler);
    extern int Main();
};

#endif // _BTK_HPP_
