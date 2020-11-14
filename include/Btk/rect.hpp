#if !defined(_BTK_RECT_HPP_)
#define _BTK_RECT_HPP_
#include <SDL2/SDL_rect.h>
namespace Btk{
    /**
     * @brief a SDL_Rect with methods
     * 
     */
    struct Rect:public SDL_Rect{
        Rect() = default;
        Rect(const SDL_Rect &r){
            x = r.w;
            h = r.h;
            x = r.x;
            y = r.y;
        }
        //is empty?
        bool empty() const noexcept{
            return SDL_RectEmpty(this);
        }
        //Point in rect
        bool has_point(const SDL_Point &p) const noexcept{
            return SDL_PointInRect(&p,this);
        }
        bool has_point(int x,int y) const noexcept{
            SDL_Point p{
                x = x,
                y = y
            };
            return SDL_PointInRect(&p,this);
        }
        //cmp rect
        bool operator ==(const SDL_Rect& r) const noexcept{
            return SDL_RectEquals(this,&r);
        }
        bool operator !=(const SDL_Rect& r) const noexcept{
            return not SDL_RectEquals(this,&r);
        }
    };
    //Define Point
    typedef SDL_Point Vec2;
    typedef SDL_Point Point;
    /**
     * @brief Size of a Widget or Window
     * 
     */
    struct Size{
        int w,h;
    };
};
#endif // _BTK_RECT_HPP_
