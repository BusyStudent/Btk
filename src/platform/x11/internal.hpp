#if !defined(_BTK_X11_INTERNAL_HPP_)
#define _BTK_X11_INTERNAL_HPP_
#include <SDL2/SDL_syswm.h>
#include <Btk/platform/x11.hpp>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define BTK_X_WINDOW(V) reinterpret_cast<XWindow>(V) 
#define BTK_X_DISPLAY(V) reinterpret_cast<XDisplay*>(V) 

#define BTK_X_CONSTANTS(V) inline constexpr auto X##V = V;
#define BTK_X_TYPE(T) typedef T X##T;

BTK_X_CONSTANTS(None);
BTK_X_CONSTANTS(True);
BTK_X_CONSTANTS(False);


BTK_X_TYPE(Status);
BTK_X_TYPE(Display);
BTK_X_TYPE(Colormap);
BTK_X_TYPE(Pixmap);
BTK_X_TYPE(Window);
BTK_X_TYPE(GC);
BTK_X_TYPE(Atom);

#undef Status

namespace Btk{
namespace X11{
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
