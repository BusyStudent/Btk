#if !defined(_BTK_X11_INTERNAL_HPP_)
#define _BTK_X11_INTERNAL_HPP_
#include <SDL2/SDL_syswm.h>
#include <Btk/platform/x11.hpp>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
namespace Btk{
namespace X11{
    using XColorMap = ::Colormap;
    using XPixMap = ::Pixmap;
    using XDisplay = ::Display;
    using XWindow = ::Window;
    using XGC = ::GC;
    using XAtom = ::Atom;
    /**
     * @brief Current Context for SDL_Window
     * 
     */
    struct XContext{
        XDisplay *display;
        XWindow window;
    };
    XContext GetXContext(SDL_Window *win);
    /**
     * @brief Handle X Error
     * 
     * @param display 
     * @param event 
     * @return int 
     */
    int XErrorHandler(Display *display,XErrorEvent *event);
}
}

#endif // _BTK_X11_INTERNAL_HPP_
