#if !defined(_BTKIMPL_UTILS_HPP_)
#define _BTKIMPL_UTILS_HPP_
//This headers provide some utils
#include "../widget.hpp"
#include "../rect.hpp"
namespace Btk{
    /**
     * @brief Calcaute a area in a Rect by alignment
     * 
     * @param r The area
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
        
        int x,y;
        switch(v_align){
            case Align::Left:
                x = rect.x;
                break;
            case Align::Right:
                x = rect.x + rect.w - w;
                break;
            case Align::Center:
                x = rect.x + rect.w / - w / 2;
                break;
            default:
                x = -1;
        }
        switch(h_align){
            case Align::Top:
                y = rect.y;
                break;
            case Align::Buttom:
                y = rect.y + rect.h - h;
                break;
            case Align::Center:
                y = rect.y + rect.w / 2 - w / 2;
                break;
            default:
                y = -1;
        }
        return Rect{
            x,
            y,
            w,
            h   
        };
    };
};

#endif // _BTKIMPL_UTILS_HPP_
