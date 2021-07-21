#if !defined(_BTK_X11_INTERNAL_HPP_)
#define _BTK_X11_INTERNAL_HPP_
#include <SDL2/SDL_syswm.h>
#include <Btk/platform/x11.hpp>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define BTK_X_WINDOW(V) reinterpret_cast<Btk::X11::XWindow>(V) 
#define BTK_X_DISPLAY(V) reinterpret_cast<Btk::X11::XDisplay*>(V) 


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
    BTKHIDDEN XContext GetXContext(SDL_Window *win);
    /**
     * @brief Handle X Error
     * 
     * @param display 
     * @param event 
     * @return int 
     */
    BTKHIDDEN int XErrorHandler(Display *display,XErrorEvent *event);
    //Value for native file dialog
    extern BTKHIDDEN bool has_zenity;
    extern BTKHIDDEN bool has_kdialog;
}
}

#endif // _BTK_X11_INTERNAL_HPP_
