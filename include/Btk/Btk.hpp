#if !defined(_BTK_HPP_)
#define _BTK_HPP_
#include "utils/template.hpp"
#include "utils/traits.hpp"
#include "window.hpp"
#include "loop.hpp"

namespace Btk{
    typedef bool(*ExceptionHandler)(std::exception*);
    BTKAPI ExceptionHandler SetExceptionHandler(ExceptionHandler);
}

#endif // _BTK_HPP_
