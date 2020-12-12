#if !defined(_BTKIMPL_UTILS_HPP_)
#define _BTKIMPL_UTILS_HPP_
//This headers provide some utils
#include <SDL2/SDL_events.h>
#include "../widget.hpp"
#include "../event.hpp"
#include "../rect.hpp"
#include "window.hpp"
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
            case Align::Buttom:
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
    KeyEvent    TranslateEvent(const SDL_KeyboardEvent &event);
    
    TextInputEvent TranslateEvent(const SDL_TextInputEvent &event);
};

#endif // _BTKIMPL_UTILS_HPP_