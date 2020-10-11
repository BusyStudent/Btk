#if !defined(_BTK_RECT_HPP_)
#define _BTK_RECT_HPP_
#include <SDL2/SDL_rect.h>
namespace Btk{
    struct Rect:public SDL_Rect{
        //is empty?
        bool empty() const noexcept{
            return SDL_RectEmpty(this);
        }
        //Point in rect
        bool has_point(const SDL_Point &p) const noexcept{
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
};
#endif // _BTK_RECT_HPP_
