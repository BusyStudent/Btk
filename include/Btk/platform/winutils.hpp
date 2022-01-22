#if !defined(_BTK_PLATFORM_WINUTILS_HPP_)
#define _BTK_PLATFORM_WINUTILS_HPP_
#include "macro.hpp"
#include "../rect.hpp"
#include "../pixels.hpp"

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_syswm.h>
//For Reparent Native Window / Move / GetInfo

#if BTK_X11
#include <X11/extensions/XShm.h>
#include <X11/Xlib.h>
namespace Btk::WinUtils{
    struct NativeHandle{
        NativeHandle() = default;
        NativeHandle(::Display *dp,::Window win){
            display = dp;
            window = win;
        }
        NativeHandle(::Window win){
            window = win;
        }
        NativeHandle(const NativeHandle &) = default;
        ~NativeHandle() = default;

        ::Display *display = {};
        ::XID      window  = {};
    };
    struct Painter{
        ::Display *display = {};
        ::Drawable drawable = {};
        ::GC       gc = {};
    };
    struct Recoder{

    };

    using NativeEvent = ::XEvent;
    //Window Utils
    inline
    bool ReparentWindow(NativeHandle parent,NativeHandle child){
        //Get display
        ::Display *display;
        if(parent.display != nullptr){
            display = parent.display;
        }
        else if(child.display != nullptr){
            display = child.display;
        }
        else{
            return false;
        }
        return ::XReparentWindow(display,child.window,parent.window,0,0);
    }
    inline
    bool MoveWindow(NativeHandle window,int x,int y){
        return ::XMoveWindow(window.display,window.window,x,y);
    }
    inline
    bool ResizeWindow(NativeHandle window,int w,int h){
        return ::XResizeWindow(window.display,window.window,w,h);
    }
    inline
    bool ShowWindow(NativeHandle window,bool show = true){
        if(show){
            return ::XMapWindow(window.display,window.window);
        }
        else{
            return ::XUnmapWindow(window.display,window.window);
        }
    }
    inline
    bool  SetWindowRect(NativeHandle window,const Rect &rect){
        bool val = true;
        val = val && XMoveWindow(window.display,window.window,rect.x,rect.y);
        val = val && XResizeWindow(window.display,window.window,rect.w,rect.h);
        return val;
    }
    inline
    Rect  GetWindowRect(NativeHandle window){
        XWindowAttributes attrs;
        XGetWindowAttributes(window.display,window.window,&attrs);
        return {
            attrs.x,
            attrs.y,
            attrs.width,
            attrs.height
        };
    }
    inline
    NativeHandle GetHandleFrom(SDL_Window *win){
        SDL_SysWMinfo info;
        SDL_GetVersion(&info.version);
        if(!SDL_GetWindowWMInfo(win,&info)){
            return {};
        }
        if(info.subsystem != SDL_SYSWM_X11){
            return {};
        }
        return {
            info.info.x11.display,
            info.info.x11.window
        };
    }
    inline 
    NativeHandle GetParentWindow(NativeHandle window){
        ::XID parent;
        if(!XQueryTree(window.display,window.window,nullptr,&parent,nullptr,nullptr)){
            return {};
        }
        return {window.display,parent};
    }
    inline 
    NativeHandle GetRootWindow(NativeHandle window){
        ::XID root;
        if(!XQueryTree(window.display,window.window,&root,nullptr,nullptr,nullptr)){
            return {};
        }
        return {window.display,root};
    }
    // inline
    // Color GetWindowPixel(NativeHandle handle,int x,int y){

    // }
    template<class Callable,class ...Args>
    bool ForChildren(NativeHandle window,Callable callable,Args &&...args){
        unsigned int nchildren;
        ::XID       *children;
        if(!XQueryTree(window.display,window.window,nullptr,nullptr,&children,&nchildren)){
            return {};
        }
        //For each
        for(unsigned int n = 0;n < nchildren;n++){
            callable(NativeHandle(window.display,children[n]),args...);
        }
        ::XFree(children);
        return true;
    }

    //DrawInterface for debug and etc...
    inline
    Painter GetPainter(NativeHandle handle){
        ::GC gc = XCreateGC(
            handle.display,
            handle.window,
            0,
            0
        );
        return {handle.display,handle.window,gc};
    }
    inline
    Painter GetPainter(SDL_Window *win){
        auto handle = GetHandleFrom(win);
        return GetPainter(handle);
    }
    inline
    void    ReleasePainter(Painter handle){
        ::XFreeGC(handle.display,handle.gc);
    }
    //Draw
    inline
    void PainterDrawRect(Painter handle,const Rect &r){
        XDrawRectangle(
            handle.display,
            handle.drawable,
            handle.gc,
            r.x,
            r.y,
            r.w,
            r.h
        );
    }
    inline
    void PainterFillRect(Painter handle,const Rect &r){
        XFillRectangle(
            handle.display,
            handle.drawable,
            handle.gc,
            r.x,
            r.y,
            r.w,
            r.h
        );
    }
    inline
    void PainterDrawPoint(Painter handle,int x,int y){
        XDrawPoint(handle.display,handle.drawable,handle.gc,x,y);
    }
    inline
    void PainterSetColor(Painter handle,Color c){
        XSetForeground(handle.display,handle.gc,MapRGBA32(c));
    }
    inline
    void PainterPresent(Painter handle){
        XFlushGC(handle.display,handle.gc);
    }
    //Recoder for Get Pixels for window / screen

}
#elif BTK_WIN32
#include <wingdi.h>

namespace Btk::WinUtils{
    typedef HWND NativeHandle;
    struct  Painter{
        HWND window;
        HDC  dc;
    };
    inline
    bool SetWindowRect(NativeHandle win,const Rect &r){
        
    }

    inline
    NativeHandle GetHandleFrom(SDL_Window *win){
        SDL_SysWMinfo info;
        SDL_GetVersion(&info.version);
        if(!SDL_GetWindowWMInfo(win,&info)){
            return {};
        }
        if(info.subsystem != SDL_SYSWM_WINDOWS){
            return {};
        }
        return info.info.win.window;
    }
    //Draw
    inline
    Painter GetPainter(NativeHandle win){
        return {win,GetDC(win)};
    }
    inline
    Painter GetPainter(SDL_Window *win){
        auto handle = GetHandleFrom(win);
        return GetPainter(handle);
    }
    inline
    void    ReleasePainter(Painter p){
        ReleaseDC(p.window,p.dc);
    }
    // inline
    // void PainterSetColor(Painter p,Color c){
        
    // }
};



#endif
namespace Btk::Platform{
    using namespace WinUtils;
}

#endif // _BTK_PLATFORM_WINUTILS_HPP_
