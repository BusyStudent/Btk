#if !defined(_BTKIMPL_UTILS_HPP_)
#define _BTKIMPL_UTILS_HPP_

#include "window.hpp"

//Internal used utils

namespace Btk{
    /**
     * @brief Alloc a temporary Window from pool
     * 
     * @param req_flags 
     * @param exclude 
     * @return WindowImpl* 
     */
    Window *AllocWindow(WindowFlags req_flags = {},WindowFlags exclude = {});
    /**
     * @brief Put a temporary Window back to pool
     * 
     * @param window 
     */
    void    FreeWindow(Window *window);
}

#endif // _BTKIMPL_UTILS_HPP_
