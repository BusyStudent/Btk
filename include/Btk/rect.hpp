#if !defined(_BTK_RECT_HPP_)
#define _BTK_RECT_HPP_
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_loadso.h>
#include <SDL2/SDL_thread.h>
#include <SDL2/SDL_filesystem.h>
#include <type_traits>
#include <iosfwd>
#include <cmath>
#include "defs.hpp"
namespace Btk{
    //ABI For SDL
    template<class Elem>
    struct _RectBase{
        _RectBase() = default;
        Elem x,y,w,h;
    };
    template<class Elem>
    struct _PointBase{
        _PointBase() = default;
        Elem x,y;
    };
    //SDL Rect and point
    template<>
    struct _RectBase<int>:public SDL_Rect{
        _RectBase() = default;
        _RectBase(const SDL_Rect &r){
            x = r.w;
            h = r.h;
            x = r.x;
            y = r.y;
        }
    };
    template<>
    struct _PointBase<int>:public SDL_Point{
        _PointBase() = default;
        _PointBase(const SDL_Point &p){
            x = p.x;
            y = p.y;
        }
    };
    //Float
    #if SDL_VERSION_ATLEAST(2,0,12)
    
    template<>
    struct _RectBase<float>:public SDL_FRect{
        _RectBase() = default;
        _RectBase(const SDL_FRect &r){
            x = r.w;
            h = r.h;
            x = r.x;
            y = r.y;
        }
    };
    template<>
    struct _PointBase<float>:public SDL_FPoint{
        _PointBase() = default;
        _PointBase(const SDL_FPoint &p){
            x = p.x;
            y = p.y;
        }
    };
    #endif
    //Rect Point Bounds forward defs
    template<class T>
    struct _Rect;
    template<class T>
    struct _Point;
    template<class T>
    struct _Bounds;

    //Rect functions forward defs
    template<class T,class P = _Point<typename T::value_type>>
    inline bool CheckIntersectRectAndLine(const T &r,const P &p1,const P &p2); 
    template<class T>
    inline T    IntersectRect(const T &r1,const T &r2);
    template<class T>
    inline T    RectCoverage(const T &r1,const T &r2);


    //Point --begin
    /**
     * @brief Basic Point
     * 
     * @tparam T 
     */
    template<class T>
    struct _Point:public _PointBase<T>{
        using value_type = T;

        using _PointBase<T>::_PointBase;
        //Construct
        _Point() = default;
        _Point(const _Point &) = default;
        _Point(T x,T y){
            this->x = x;
            this->y = y;
        }
        //Autocast
        template<class Elem>
        _Point(const _Point<Elem> &p){
            this->x = static_cast<T>(p.x);
            this->y = static_cast<T>(p.y);
        }
        //Method
        template<class Elem>
        _Point<Elem> cast() const{
            return *this;
        }
        /**
         * @brief Get distance of another point
         * 
         * @param a2 
         * @return T 
         */
        T distance(const _Point &a2) const noexcept{
            T a = (this->x - a2.x) * (this->x - a2.x);
            T b = (this->y - a2.y) * (this->y - a2.y);
            return std::sqrt(a + b);
        }
        T norm() const noexcept{
            return std::sqrt(this->x * this->x + this->y * this->y);
        }
        //Transform
        _Point<T> translate(T x,T y) const noexcept{
            return {
                this->x + x,
                this->y + y
            };
        }
        template<class Real>
        _Point<T> scale(Real x_f,Real y_f) const noexcept{
            return {
                this->x * x_f,
                this->y * y_f,
            };
        }

        //Compare
        bool operator ==(const _Point &p) const noexcept{
            return this->x == p.x and this->y == p.y;
        }
        bool operator !=(const _Point &p) const noexcept{
            return not operator ==(p);
        }
        
        //calculation
        _Point<T> operator -(const _Point &p) const noexcept{
            return _Point<T>(this->x - p.x,this->y - p.y);
        }
        _Point<T> operator +(const _Point &p) const noexcept{
            return _Point<T>(this->x + p.x,this->y + p.y);
        }
        T operator *(const _Point &p) const noexcept{
            return this->x * p.x + this->y * p.y;
        }
        T operator ^(const _Point &p) const noexcept{
            return this->x * p.y - this->y * p.x;
        }

    };
    //Point --end
    /**
     * @brief Size of a Widget or Window
     * 
     */
    template<class T>
    struct _Size{
        using value_type = T;

        T w,h;
        //Construct
        _Size() = default;
        _Size(const _Size &) = default;
        _Size(T w,T h){
            this->w = w;
            this->h = h;
        }
        //Auto cast
        template<class Elem>
        _Size(const _Size<Elem> &size){
            w = static_cast<T>(size.w);
            h = static_cast<T>(size.h);
        }
        //Method
        template<class Elem>
        _Size<Elem> cast() const{
            return *this;
        }


        //Compare
        bool operator ==(const _Size &size) const noexcept{
            return w == size.w and h == size.h;
        }
        bool operator !=(const _Size &size) const noexcept{
            return w != size.w or h != size.h;
        }
    };
    using Size = _Size<int>;
    using FSize = _Size<float>;

    //Rect --begin
    /**
     * @brief Basic Rect
     * 
     * @tparam T 
     */
    template<class T>
    struct _Rect:public _RectBase<T>{
        using value_type = T;
        //Construct
        //Using parent constructor
        using _RectBase<T>::_RectBase;

        _Rect() = default;
        _Rect(const _Rect&) = default;
        _Rect(T x,T y,T w,T h){
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
        }
        //From another Elem rect
        template<class Elem>
        _Rect(const _Rect<Elem> &rect){
            this->x = static_cast<T>(rect.x);
            this->y = static_cast<T>(rect.y);
            this->w = static_cast<T>(rect.w);
            this->h = static_cast<T>(rect.h);
        }
        //Method
        template<class Elem>
        _Rect<Elem> cast() const noexcept{
            return *this;
        }
        //Has point
        bool has_point(T px,T py) const noexcept{
            return  px > this->x 
                and px < (this->x + this->w) 
                and py > this->y 
                and py < (this->y + this->h);
        }
        template<class Elem>
        bool has_point(const _Point<Elem> &point) const noexcept{
            return has_point(
                static_cast<T>(point.x),
                static_cast<T>(point.y)
            );
        }
        //Get center
        template<class Elem = T>
        _Point<Elem> center() const noexcept{   
            Elem _x = Elem(this->x) + Elem(this->w) / 2;
            Elem _y = Elem(this->y) + Elem(this->h) / 2;
            return {_x,_y};
        }
        template<class Elem = T>
        Elem hcenter() const noexcept{   
            Elem _x = Elem(this->x) + Elem(this->w) / 2;
            return _x;
        }
        template<class Elem = T>
        Elem vcenter() const noexcept{   
            Elem _y = Elem(this->y) + Elem(this->h) / 2;
            return _y;
        }
        //Intersect
        _Rect<T> intersect_with(_Rect<T> &r) const noexcept{
            return IntersectRect(*this,r);
        }
        bool     has_intersection(_Rect<T> &r) const noexcept{
            return not intersect_with(r).empty();
        }
        //Transform
        _Rect<T> translate(T x,T y) const noexcept{
            return {
                this->x + x,
                this->y + y,
                this->w,
                this->h
            };
        }
        /**
         * @brief Scale rect's width and height
         * 
         * @param w_f 
         * @param h_f 
         * @return _Rect<T> 
         */
        template<class Real>
        _Rect<T> scale(Real w_f,Real h_f) const noexcept{
            return {
                this->x,
                this->y,
                this->w * w_f,
                this->h * h_f
            };
        }
        /**
         * @brief Get Size of the Rect
         * 
         * @tparam Elem 
         * @return _Size<Elem> 
         */
        template<class Elem = T>
        _Size<Elem> size() const noexcept{
            return {
                static_cast<Elem>(this->w),
                static_cast<Elem>(this->h)
            };
        }
        //Is empty?
        bool empty() const noexcept{
            return this->w <= 0 or this->h <= 0;
        }

        //Compare
        bool operator ==(const _Rect &rect) const noexcept{
            return  this->x == rect.x 
                and this->y == rect.y
                and this->w == rect.w
                and this->h == rect.h;
        }
        bool operator !=(const _Rect &rect) const noexcept{
            return not operator ==(rect);
        }

    };
    //Rect --end
    using Rect = _Rect<int>;
    using FRect = _Rect<float>;

    using Point = _Point<int>;
    using FPoint = _Point<float>;

    using Vec2 = Point;
    using FVec2 = FPoint;

    //Line --begin
    template<class T>
    struct _Line{
        using value_type = T;

        T x1,y1;
        T x2,y2;
        //Construct
        _Line() = default;
        _Line(const _Line &) = default;
        _Line(T x1,T y1,T x2,T y2){
            this->x1 = x1;
            this->y1 = y1;
            this->x2 = x2;
            this->y2 = y2;
        }
        //Autocast
        template<class Elem>
        _Line(Elem x1,Elem y1,Elem x2,Elem y2){
            this->x1 = static_cast<T>(x1);
            this->y1 = static_cast<T>(y1);
            this->x2 = static_cast<T>(x2);
            this->y2 = static_cast<T>(y2);
        }

        _Point<T> p1() const noexcept{
            return {x1,x2};   
        }
        _Point<T> p2() const noexcept{
            return {x2,y2};
        }

    };
    //Line --end

    using FLine = _Line<float>;

    //Bounds --begin
    template<class T>
    struct _Bounds{
        using value_type = T;

        T minx;//Min x
        T miny;//Min y
        T maxx;//Max x
        T maxy;//Max y
        //Construct
        _Bounds() = default;
        _Bounds(const _Bounds &) = default;
        _Bounds(T minx,T miny,T maxx,T maxy){
            this->minx = minx;
            this->miny = miny;
            this->maxx = maxx;
            this->maxy = maxy;
        }
        //Autocast
        template<class Elem>
        _Bounds(const _Bounds<Elem> &bounds){
            minx = static_cast<T>(bounds.minx);
            miny = static_cast<T>(bounds.miny);
            maxx = static_cast<T>(bounds.maxx);
            maxy = static_cast<T>(bounds.maxy);
        }
        /**
         * @brief Construct a new bounds object from Rect
         * 
         * @tparam Elem 
         * @param rect 
         */
        template<class Elem>
        _Bounds(const _Rect<Elem> &rect){
            minx = static_cast<T>(rect.x);
            miny = static_cast<T>(rect.y);
            maxx = static_cast<T>(rect.x + rect.w);
            maxy = static_cast<T>(rect.y + rect.h);
        }
        //Cast method
        /**
         * @brief Cast to Rect
         * 
         * @tparam Rect The rect type
         * @return _Rect<Elem> 
         */
        template<class Rect,class Elem = typename Rect::value_type>
        _Rect<Elem> cast() const noexcept{
            return {
                minx,
                miny,
                maxx - minx,
                maxy - miny,
            };
        }
        /**
         * @brief Cast to another bounds
         * 
         * @tparam Elem 
         */
        template<
            class Elem,
            typename _Cond = std::enable_if_t<std::is_arithmetic_v<Elem>>
        >
        _Bounds<Elem> cast() const noexcept{
            return *this;
        }
        //Method
        _Point<T> min() const{
            return {minx,miny};
        }
        _Point<T> max() const{
            return {maxx,maxy};
        }
        bool empty() const noexcept{
            return maxx > minx or maxy < miny;
        }
        //Compare
        bool operator ==(const _Bounds &b) const noexcept{
            return  maxx == b.maxx 
                and maxy == b.maxy
                and minx == b.minx
                and miny == b.miny;
        }
        bool operator !=(const _Bounds &b) const noexcept{
            return not operator ==(b);
        }
        //Helper
        /**
         * @brief Construct a new Rect from bounds
         * 
         * @tparam Elem 
         */
        template<class Elem>
        operator _Rect<Elem>() const noexcept{
            return cast<_Rect<Elem>>();
        }

        static _Bounds<T> FromPoints(const _Point<T> &min,const _Point<T> &max){
            return {
                min.x,
                min.y,
                max.x,
                max.y
            };
        }
    };

    using Bounds = _Bounds<int>;
    using FBounds = _Bounds<float>;
    //Bounds --end

    //Template alias for User
    template<class T>
    using RectImpl = _Rect<T>;
    template<class T>
    using SizeImpl = _Size<T>;
    template<class T>
    using LineImpl = _Line<T>;
    template<class T>
    using PointImpl = _Point<T>;
    template<class T>
    using BoundsImpl = _Bounds<T>;

    //Utils
    template<class T1,class T2,class T3>
    inline bool PointInCircle(const _Point<T1> &center,T2 r,const _Point<T3> &point){
        return center.distance(point) <= r;
    }
    //eps
    inline const double eps = 1e-8;
    inline int _sgnCheck_(double x){
        if(fabs(x) < eps) return 0;
        if(x < 0) return -1;
        return 1;
    }
    template<class P>
    inline bool _segCrossSeg_(P p1,P p2,P p3,P p4){
        int d1 = _sgnCheck_((p2-p1)^(p3-p1));
        int d2 = _sgnCheck_((p2-p1)^(p4-p1));
        int d3 = _sgnCheck_((p4-p3)^(p1-p3));
        int d4 = _sgnCheck_((p4-p3)^(p2-p3));

        if ((d1^d2) == -2 && (d3^d4) == -2) return true;
        return (d1 == 0 && _sgnCheck_((p3 - p1) * (p3 - p2)) <= 0) ||
               (d2 == 0 && _sgnCheck_((p4 - p1) * (p4 - p2)) <= 0) ||
               (d3 == 0 && _sgnCheck_((p1 - p3) * (p1 - p4)) <= 0) ||
               (d4 == 0 && _sgnCheck_((p2 - p3) * (p2 - p4)) <= 0);
    }
    //Rect utils
    /**
     * @brief Judge intersection of line to rect 
     * 
     * @tparam T
     * @param r
     * @param p1
     * @param p2
     * @return true or false
     */
    template<class T,class P = _Point<typename T::value_type>>
    inline bool CheckIntersectRectAndLine(const T &r,const P &p1,const P &p2){
        P leftTop = P(r.x, r.y);
        P rightDown = P(r.x + r.w, r.y + r.h);
        P leftDown = P(r.x, r.y + r.h);
        P rightTop = P(r.x + r.w, r.y);
        SDL_Log("p1 = {%d,%d}",p1.x,p1.y);
        SDL_Log("p2 = {%d,%d}",p2.x,p2.y);
        SDL_Log("leftTop = {%d,%d}",leftTop.x,leftTop.y);
        SDL_Log("rightDown = {%d,%d}",rightDown.x,rightDown.y);
        SDL_Log("leftDown = {%d,%d}",leftDown.x,leftDown.y);
        SDL_Log("rightTop = {%d,%d}",rightTop.x,rightTop.y);
        bool d1 = _segCrossSeg_(p1,p2,leftTop,rightTop);
        bool d2 = _segCrossSeg_(p1,p2,rightTop,rightDown);
        bool d3 = _segCrossSeg_(p1,p2,rightDown,leftDown);
        bool d4 = _segCrossSeg_(p1,p2,leftDown,leftTop);

        SDL_Log("d1 = %d d2 = %d d3 = %d d4 = %d",d1,d2,d3,d4);

        return d1 || d2 || d3 || d4;
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

    /**
     *@brief Rects Coverage
     * 
     * @tparam T rect
     * @param r1 rect1
     * @param r2 rect2
     * @return rect coverage rects
     */
    template<class T>
    inline T RectCoverage(const T &r1,const T &r2){
        T rect;
        rect.x = min(r1.x,r2.x);
        rect.y = min(r1.y,r2.y);
        rect.w = max(r1.x + r1.w, r2.x + r2.w) - rect.x;
        rect.h = max(r1.y + r1.h, r2.y + r2.h) - rect.y;
        return rect;
    }
    //Do extern template
    // extern template BTKAPI FRect IntersectRect<FRect>(const FRect &,const FRect &);

    BTKAPI std::ostream &operator <<(std::ostream&,const Rect &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FRect &);
    BTKAPI std::ostream &operator <<(std::ostream&,const Size &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FSize &);
    BTKAPI std::ostream &operator <<(std::ostream&,const Vec2 &);
    BTKAPI std::ostream &operator <<(std::ostream&,const FVec2 &);
}
#endif // _BTK_RECT_HPP_
