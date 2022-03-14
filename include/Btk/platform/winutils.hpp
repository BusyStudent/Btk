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
    struct Recorder{

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
    //Recorder for Get Pixels for window / screen

}
#elif BTK_WIN32
#include <wingdi.h>

namespace Btk::WinUtils{
    typedef HWND NativeHandle;
    struct  Painter{
        HWND window;
        HDC  dc;
    };
    /**
     * @brief A struct for capturing window pixels by using GDI
     * 
     */
    struct Recorder{
        HWND window;
        HDC  windc;//< Out source DC
        HDC  memdc;//< Our destination DC
        HBITMAP bitmap;//< Cached bitmap
        int bitmap_w;
        int bitmap_h;
    };
    inline
    bool SetWindowRect(NativeHandle win,const Rect &r){
        return ::MoveWindow(win,r.x,r.y,r.w,r.h,true);
    }
    inline
    Rect GetWindowRect(NativeHandle win){
        RECT rect;
        ::GetWindowRect(win,&rect);
        return {
            rect.left,
            rect.top,
            rect.right - rect.left,
            rect.bottom - rect.top
        };
    }

    inline
    bool ReparentWindow(NativeHandle parent,NativeHandle child){
        return ::SetParent(child,parent);
    }

    inline
    NativeHandle GetParentWindow(NativeHandle win){
        return ::GetParent(win);
    }
    inline
    NativeHandle GetRootWindow(NativeHandle win){
        return ::GetAncestor(win,GA_ROOT);
    }
    using ::GetDesktopWindow;

    template<class Callable,class ...Args>
    bool ForChildren(NativeHandle win,Callable callable,Args &&...args){
        //Get all children
        HWND child = ::GetWindow(win,GW_CHILD);
        while(child){
            callable(child,args...);
            child = ::GetWindow(child,GW_HWNDNEXT);
        }
        return true;
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
    inline
    Recorder GetRecoder(NativeHandle win){
        Recorder recorder;
        recorder.window = win;
        recorder.windc = GetDC(win);
        recorder.memdc = CreateCompatibleDC(recorder.windc);
        recorder.bitmap = nullptr;
        recorder.bitmap_w = 0;
        recorder.bitmap_h = 0;
        return recorder;
    }
    inline
    void RecorderQuery(Recorder &recorder,int *w,int *h){
        RECT win_rect;
        ::GetWindowRect(recorder.window,&win_rect);
        //Check pointer
        if(w){
            *w = win_rect.right - win_rect.left;
        }
        if(h){
            *h = win_rect.bottom - win_rect.top;
        }
    }
    /**
     * @brief Read Pixels from Recorder
     * 
     * @param recorder 
     * @note In windows the rgb is a BGR(so do not use it unless you know what you are doing)
     * @param pixels The Win32 RGB pixels(4 bytes per pixel)
     */
    inline
    void RecorderReadInternal(Recorder &recorder,void *pixels){
        RECT win_rect;
        ::GetWindowRect(recorder.window,&win_rect);
        int win_w = win_rect.right - win_rect.left;
        int win_h = win_rect.bottom - win_rect.top;

        //Cretae a bitmap info
        BITMAPINFO bmi;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = win_w;
        bmi.bmiHeader.biHeight = -(win_h);
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 0;
        bmi.bmiHeader.biXPelsPerMeter = 0;
        bmi.bmiHeader.biYPelsPerMeter = 0;
        bmi.bmiHeader.biClrUsed = 0;
        bmi.bmiHeader.biClrImportant = 0;
        //Read pixels
        //Check bitmap size is same as last time
        if(recorder.bitmap_h != win_h or recorder.bitmap_w != win_w){
            ::DeleteObject(recorder.bitmap);
            recorder.bitmap = nullptr;
            recorder.bitmap_w = win_w;
            recorder.bitmap_h = win_h;
        }
        if(recorder.bitmap == nullptr){
            recorder.bitmap = ::CreateCompatibleBitmap(
                recorder.windc,
                win_w,
                win_h
            );
        }
        //Do bitblt
        ::SelectObject(recorder.memdc,recorder.bitmap);
        ::BitBlt(
            recorder.memdc,
            0,
            0,
            win_w,
            win_h,
            recorder.windc,
            0,
            0,
            SRCCOPY
        );
        //Get pixels
        ::GetDIBits(
            recorder.memdc,
            recorder.bitmap,
            0,
            win_h,
            pixels,
            &bmi,
            DIB_RGB_COLORS
        );
    }
    /**
     * @brief Read RGBA pixels from Recorder
     * 
     * @param recorder 
     * @param pixels The RGBA pixels(4 bytes per pixel)
     */
    inline
    void RecorderReadRGBA(Recorder &recorder,void *pixels){
        RecorderReadInternal(recorder,pixels);
        //We need to convert the pixels to RGBA
        auto p = (Uint32 *)pixels;
        int w = recorder.bitmap_w;
        int h = recorder.bitmap_h;

        for(int i = 0;i < w * h;i++){
            Uint32 pixel = p[i];
            //Get BGR
            Uint8 b = GetRValue(pixel);
            Uint8 g = GetGValue(pixel);
            Uint8 r = GetBValue(pixel);
            //Set RGBA
            p[i] = MapRGBA32(r,g,b,255);
        }
    }

    inline
    void ReleaseRecoder(Recorder recorder){
        ::DeleteObject(recorder.bitmap);
        ::DeleteDC(recorder.memdc);
        ::ReleaseDC(recorder.window,recorder.windc);
    }
    
};



#endif
namespace Btk::Platform{
    using namespace WinUtils;
}

#endif // _BTK_PLATFORM_WINUTILS_HPP_
