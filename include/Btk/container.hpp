#if !defined(_BTK_CONTAINER_HPP_)
#define _BTK_CONTAINER_HPP_
/**
 * @brief This header include many useful containers
 * 
 */
#include "defs.hpp"
#include "widget.hpp"
namespace Btk{
    /**
     * @brief A Interface for manage widget
     * 
     */
    class BTKAPI Group:public Container{
        public:
            Group() = default;
            ~Group() = default;
        private:
            //< The current dragging widget
            Widget *dragg_widget = nullptr;
            Widget *focus_widget = nullptr;
            Widget *mouse_widget = nullptr;
            bool drag_rejected = false;
    };
    class BTKAPI GroupBox:public Group{

    };
    class BTKAPI DockWidget:public Group{
        
    };
}

#endif // _BTK_CONTAINER_HPP_
