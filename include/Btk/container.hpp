#if !defined(_BTK_CONTAINER_HPP_)
#define _BTK_CONTAINER_HPP_
/**
 * @brief This header include many useful containers
 * 
 */
#include "defs.hpp"
#include "widget.hpp"
namespace Btk{

    class BTKAPI GroupBox:public Widget,Container{

    };
    class BTKAPI DockWidget:public Widget,Container{
        
    };
}

#endif // _BTK_CONTAINER_HPP_
