#if !defined(_BTKIMPL_UTILS_HPP_)
#define _BTKIMPL_UTILS_HPP_
//This headers provide some utils
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_endian.h>
#include "../utils/mem.hpp"
#include "../string.hpp"
#include "../widget.hpp"
#include "../event.hpp"
#include "../rect.hpp"
#include "window.hpp"
#include <string>
namespace Btk{
    /**
     * @brief Calcaute a area's X in a Rect by alignment
     * 
     * @param rect The rect
     * @param w The W
     * @param v_align V alignment
     * @return -1 if failed
     */
    inline int CalculateXByAlign(const Rect &rect,int w,Align v_align){
        switch(v_align){
            case Align::Left:
                return rect.x;
            case Align::Right:
                return rect.x + rect.w - w;
            case Align::Center:
                return rect.x + ((rect.w - w) / 2);
            default:
                return -1;
        }
    };
    /**
     * @brief Calcaute a area's Y in a Rect by alignment
     * 
     * @param rect The rect
     * @param h The height
     * @param h_align H alignment
     * @return -1 if failed
     */
    inline int CalculateYByAlign(const Rect &rect,int h,Align h_align){
        switch(h_align){
            case Align::Top:
                return rect.y;
            case Align::Bottom:
                return rect.y + rect.h - h;
            case Align::Center:
                return rect.y + ((rect.h - h) / 2);
            default:
                return -1;
        }
    };
    /**
     * @brief Calcaute a area in a Rect by alignment
     * 
     * @param rect The Rect
     * @param w area w
     * @param h area h
     * @param v_align V alignment
     * @param h_align H alignment
     * @return aligned rect empty() if failed
     */
    inline Rect CalculateRectByAlign
        (const Rect &rect,
         int w,int h,
         Align v_align,
         Align h_align){
        
        
        return Rect{
            CalculateXByAlign(rect,w,v_align),
            CalculateYByAlign(rect,h,h_align),
            w,
            h   
        };
    };
    //Translate SDL_Event to Btk's event
    MotionEvent TranslateEvent(const SDL_MouseMotionEvent &event);
    MouseEvent  TranslateEvent(const SDL_MouseButtonEvent &event);
    WheelEvent  TranslateEvent(const SDL_MouseWheelEvent &event);
    DropEvent   TranslateEvent(const SDL_DropEvent       &event);
    KeyEvent    TranslateEvent(const SDL_KeyboardEvent &event);
    
    TextInputEvent TranslateEvent(const SDL_TextInputEvent &event);
    /**
     * @brief Internal Utf8 string buffer(thread_local)
     * 
     * @return BTKAPI& 
     */
    BTKAPI u8string& InternalU8Buffer();
    /**
     * @brief Internal Utf16 string buffer(thread_local)
     * 
     * @return BTKAPI& 
     */
    BTKAPI u16string& InternalU16Buffer();
    /**
     * @brief Fill the internal u8buffer(thread_local)
     * 
     * @param view 
     * @return std::string& 
     */
    inline u8string& FillInternalU8Buffer(u8string_view view){
        auto &buf = InternalU8Buffer();
        buf = view;
        return buf;
    }
    inline u8string& FillInternalU8Buffer(u8string &&text){
        auto &buf = InternalU8Buffer();
        buf = std::move(text);
        return buf;
    }
    /**
     * @brief Fill the internal u8buffer(thread_local)
     * 
     * @param view 
     * @return std::string& 
     */
    inline u8string& FillInternalU8Buffer(u16string_view view){
        auto &buf = InternalU8Buffer();
        buf.clear();
        Utf16To8(buf,view);
        return buf;
    }
    inline Uint32 MapRGBA32(Color c){
        Uint32 pixel;
        #if SDL_BYTEORDER == SDL_BIG_ENDIAN
        //Big endian RGBA8888
        reinterpret_cast<Uint8*>(&pixel)[0] = c.a;
        reinterpret_cast<Uint8*>(&pixel)[1] = c.b;
        reinterpret_cast<Uint8*>(&pixel)[2] = c.g;
        reinterpret_cast<Uint8*>(&pixel)[3] = c.r;
        #else
        //little endian ABGR8888
        reinterpret_cast<Uint8*>(&pixel)[0] = c.r;
        reinterpret_cast<Uint8*>(&pixel)[1] = c.g;
        reinterpret_cast<Uint8*>(&pixel)[2] = c.b;
        reinterpret_cast<Uint8*>(&pixel)[3] = c.a;
        #endif
        return pixel;

    }
    inline Uint32 MapRGBA32(Uint8 r,Uint8 g,Uint8 b,Uint8 a = 255){
        return MapRGBA32({r,g,b,a});
    }
};

#endif // _BTKIMPL_UTILS_HPP_
