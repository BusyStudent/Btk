#if !defined(_BTK_RECT_HPP_)
#define _BTK_RECT_HPP_
#include <SDL2/SDL_rect.h>
#include <iosfwd>
#include <cmath>
#include "defs.hpp"
namespace Btk{
    struct Point;
    struct FPoint;
    struct Rect;
    struct FRect;
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
        Rect(int x,int y,int w,int h){
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
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
                x,
                y
            };
            return SDL_PointInRect(&p,this);
        }
        /**
         * @brief Get center
         * 
         * @return FPoint 
         */
        FPoint center() const noexcept;
        //cmp rect
        bool operator ==(const SDL_Rect& r) const noexcept{
            return SDL_RectEquals(this,&r);
        }
        bool operator !=(const SDL_Rect& r) const noexcept{
            return not SDL_RectEquals(this,&r);
        }
    };
    struct FPoint{
        FPoint() = default;
        FPoint(const FPoint &) = default;
        FPoint(float x,float y){
            this->x = x;
            this->y = y;
        }
        /**
         * @brief Get distance of a another point
         * 
         * @param a2 
         * @return float 
         */
        float distance(const FPoint &a2) const noexcept{
            float a = (x - a2.x) * (x - a2.x);
            float b = (y - a2.y) * (y - a2.y);
            return std::sqrt(a + b);
        }
        float norm() const noexcept{
            return std::sqrt(x * x + y * y);
        }
        bool operator ==(const FPoint &p) const noexcept{
            return x == p.x and y == p.y;
        }
        bool operator !=(const FPoint &p) const noexcept{
            return x != p.x or y != p.y;
        }

        float x,y;
    };
    struct FRect{
        FRect() = default;
        FRect(const FRect &) = default;
        FRect(const Rect &r){
            x = r.x;
            y = r.y;
            w = r.w;
            h = r.h;
        }
        FRect(float x,float y,float w,float h){
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
        }
        float x, y;
        float w, h;
        
        bool empty() const noexcept{
            return w <= 0 or h <= 0;
        }
        bool has_point(const FPoint &fp){
            return has_point(fp.x,fp.y);
        }
        bool has_point(float px,float py) const noexcept{
            if(px > x and px < (x + w) and py > y and py < (y + h)){
                return true;
            }
            return false;
        }
        FPoint center() const noexcept;
        operator Rect() const noexcept{
            return Rect{
                static_cast<int>(x),
                static_cast<int>(y),
                static_cast<int>(w),
                static_cast<int>(h)
            };
        }
        bool operator ==(const FRect &r){
            return x == r.x and y == r.y and w == r.w and h == r.h;
        }
        bool operator !=(const FRect &r){
            return not operator ==(r);
        }
    };
    //Define Point
    struct Point:public SDL_Point{
        Point() = default;
        Point(int x,int y){
            this->x = x;
            this->y = y;
        }
        Point(const SDL_Point &p){
            x = p.x;
            y = p.y;
        }
        Point(const FPoint &fp){
            x = float(fp.x);
            y = float(fp.y);
        }
        bool operator ==(const SDL_Point &p) const noexcept{
            return x == p.x and y == p.y;
        }
        bool operator !=(const SDL_Point &p) const noexcept{
            return x != p.x or y != p.y;
        }
        operator FPoint() const noexcept{
            return {float(x),float(y)};
        }
        float distance(const FPoint &p) const noexcept{
            return FPoint(*this).distance(p);
        }
    };
    typedef Point  Vec2;
    typedef FPoint FVec2;
    /**
     * @brief Size of a Widget or Window
     * 
     */
    struct Size{
        int w;
        int h;

        bool operator ==(const Size &s) const noexcept{
            return w == s.w and h == s.h;
        }
        bool operator !=(const Size &s) const noexcept{
            return not operator ==(s);
        }
    };
    /**
     * @brief Float size
     * 
     */
    struct FSize{
        float w;
        float h;

        bool operator ==(const FSize &s) const noexcept{
            return w == s.w and h == s.h; 
        }
        bool operator !=(const FSize &s) const noexcept{
            return not operator ==(s);
        }
        operator Size() const noexcept{
            return {
                int(w),
                int(h)
            };
        }
    };
    inline FPoint FRect::center() const noexcept{
        float _x = x + w / 2;
        float _y = y + h / 2;
        return {_x,_y};
    }
    inline FPoint Rect::center() const noexcept{
       return FRect(*this).center();
    }

    //Utils
    inline bool PointInCircle(const FPoint &center,float r,const FPoint &point){
        return center.distance(point) <= r;
    }
    /**
     * @brief Get intersection of two rects
     * 
     * @tparam T 
     * @param r1 
     * @param r2 
     * @return intersection
     */
    template<class T>
    inline T IntersectRect(const T &r1,const T &r2){
        auto minx = max(r1.x,r2.x);
        auto miny = max(r1.y,r2.y);
        auto maxx = min(r1.x + r1.w,r2.x + r2.w);
        auto maxy = min(r1.y + r1.h,r2.y + r2.h);
        return T(
            minx,
            miny,
            max(static_cast<decltype(minx)>(0),maxx - minx),
            max(static_cast<decltype(minx)>(0),maxy - miny)
        );
    }

    BTKAPI std::ostream &operator <<(std::ostream&,const Rect &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FRect &);
    BTKAPI std::ostream &operator <<(std::ostream&,const Size &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FSize &);
    BTKAPI std::ostream &operator <<(std::ostream&,const Vec2 &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FVec2 &);
}
#endif // _BTK_RECT_HPP_
