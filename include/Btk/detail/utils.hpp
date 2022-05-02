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
    WindowImpl *AllocWindow(WindowFlags req_flags = {},WindowFlags exclude = {});
    /**
     * @brief Free a Window from pool
     * 
     * @param window 
     */
    void        FreeWindow(WindowImpl *window);
}

#endif // _BTKIMPL_UTILS_HPP_
