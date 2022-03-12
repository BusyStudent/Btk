#if !defined(_BTK_RECT_HPP_)
#define _BTK_RECT_HPP_
#include <SDL2/SDL_version.h>
#include <SDL2/SDL_rect.h>
#include <type_traits>
#include <iosfwd>
#include <vector> //> For _Polygen
#include <cmath>
#include "defs.hpp"
namespace Btk{
    //ABI For SDL
    template<class Elem>
    struct _RectBase{
        _RectBase() noexcept = default;
        Elem x,y,w,h;
    };
    template<class Elem>
    struct _PointBase{
        _PointBase() noexcept = default;
        Elem x,y;
    };
    //SDL Rect and point
    template<>
    struct _RectBase<int>:public SDL_Rect{
        _RectBase() noexcept = default;
        _RectBase(const SDL_Rect &r) noexcept{
            x = r.w;
            h = r.h;
            x = r.x;
            y = r.y;
        }
    };
    template<>
    struct _PointBase<int>:public SDL_Point{
        _PointBase() noexcept = default;
        _PointBase(const SDL_Point &p) noexcept{
            x = p.x;
            y = p.y;
        }
    };
    //Float
    #if SDL_VERSION_ATLEAST(2,0,12)
    
    template<>
    struct _RectBase<float>:public SDL_FRect{
        _RectBase() noexcept = default;
        _RectBase(const SDL_FRect &r) noexcept{
            x = r.w;
            h = r.h;
            x = r.x;
            y = r.y;
        }
    };
    template<>
    struct _PointBase<float>:public SDL_FPoint{
        _PointBase() noexcept = default;
        _PointBase(const SDL_FPoint &p) noexcept{
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
    template<class T>
    struct _Polygen;

    //Rect functions forward defs
    template<class T,class P = _Point<typename T::value_type>>
    inline bool CheckIntersectRectAndLine(const T &r,const P &p1,const P &p2); 
    template<class T>
    inline T    IntersectRect(const T &r1,const T &r2) noexcept;
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
        _Point() noexcept = default;
        _Point(const _Point &) noexcept = default;
        _Point(T x,T y) noexcept{
            this->x = x;
            this->y = y;
        }
        //Autocast
        template<class Elem>
        _Point(const _Point<Elem> &p) noexcept{
            this->x = static_cast<T>(p.x);
            this->y = static_cast<T>(p.y);
        }
        //Method
        template<class Elem>
        _Point<Elem> cast() const noexcept{
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
        _Size() noexcept = default;
        _Size(const _Size &) noexcept = default;
        _Size(T w,T h) noexcept{
            this->w = w;
            this->h = h;
        }
        //Auto cast
        template<class Elem>
        _Size(const _Size<Elem> &size) noexcept{
            w = static_cast<T>(size.w);
            h = static_cast<T>(size.h);
        }
        //Method
        template<class Elem>
        _Size<Elem> cast() const noexcept{
            return *this;
        }

        bool invalid() const noexcept{
            return w < 0 or h < 0;
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

        _Rect() noexcept = default;
        _Rect(const _Rect&) noexcept = default;
        _Rect(T x,T y,T w,T h) noexcept{
            this->x = x;
            this->y = y;
            this->w = w;
            this->h = h;
        }
        //From another Elem rect
        template<class Elem>
        _Rect(const _Rect<Elem> &rect) noexcept{
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
        _Line() noexcept = default;
        _Line(const _Line &) noexcept = default;
        _Line(T x1,T y1,T x2,T y2) noexcept{
            this->x1 = x1;
            this->y1 = y1;
            this->x2 = x2;
            this->y2 = y2;
        }
        //Autocast
        template<class Elem>
        _Line(Elem x1,Elem y1,Elem x2,Elem y2) noexcept{
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

        T length() const noexcept{
            return p1().distance(p2());
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
        _Bounds() noexcept = default;
        _Bounds(const _Bounds &) noexcept = default;
        _Bounds(T minx,T miny,T maxx,T maxy) noexcept{
            this->minx = minx;
            this->miny = miny;
            this->maxx = maxx;
            this->maxy = maxy;
        }
        //Autocast
        template<class Elem>
        _Bounds(const _Bounds<Elem> &bounds) noexcept{
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
        _Bounds(const _Rect<Elem> &rect) noexcept{
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
        _Point<T> min() const noexcept{
            return {minx,miny};
        }
        _Point<T> max() const noexcept{
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

        static _Bounds<T> FromPoints(const _Point<T> &min,const _Point<T> &max) noexcept{
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

    //Margin --begin
    /**
     * @brief A margin struct
     * 
     * @tparam T The element type
     */
    template<class T>
    struct _Margin{
        using value_type = T;

        T left = {};
        T top = {};
        T right = {};
        T bottom = {};
        /**
         * @brief Construct a new margin object(init all value to 0)
         * 
         */
        _Margin() noexcept = default;
        _Margin(const _Margin &) noexcept = default;
        _Margin(T left,T top,T right,T bottom) noexcept{
            this->left = left;
            this->top = top;
            this->right = right;
            this->bottom = bottom;
        }
        //Autocast
        template<class Elem>
        _Margin(Elem left,Elem top,Elem right,Elem bottom) noexcept{
            this->left = static_cast<T>(left);
            this->top = static_cast<T>(top);
            this->right = static_cast<T>(right);
            this->bottom = static_cast<T>(bottom);
        }
        template<class Elem>
        _Margin(const _Margin<Elem> &margin) noexcept{
            left = static_cast<T>(margin.left);
            top = static_cast<T>(margin.top);
            right = static_cast<T>(margin.right);
            bottom = static_cast<T>(margin.bottom);
        }

        //Compare
        bool compare(const _Margin &m) const noexcept{
            return  left == m.left 
                and top == m.top
                and right == m.right
                and bottom == m.bottom;
        }
        //Test
        bool empty() const noexcept{
            return left == 0 and top == 0 and right == 0 and bottom == 0;
        }
        bool invalid() const noexcept{
            return left < 0 or top < 0 or right < 0 or bottom < 0;
        }
        /**
         * @brief Set all value to args
         * 
         * @param v 
         * @return _Margin 
         */
        _Margin &set(T v) noexcept{
            left = v;
            top = v;
            right = v;
            bottom = v;
            return *this;
        }
        //Scale
        _Margin scale(T scale) const noexcept{
            return {
                left * scale,
                top * scale,
                right * scale,
                bottom * scale,
            };
        }
        //Operator
        bool operator ==(const _Margin &m) const noexcept{
            return compare(m);
        }
        bool operator !=(const _Margin &m) const noexcept{
            return not operator ==(m);
        }
        /**
         * @brief Apply margin to bounds
         * 
         */
        template<class Elem>
        _Bounds<Elem> apply(const _Bounds<Elem> &bounds) const noexcept{
            return {
                bounds.minx + left,
                bounds.miny + top,
                bounds.maxx - right,
                bounds.maxy - bottom
            };
        }
        /**
         * @brief Apply margin to rect
         * 
         */
        template<class Elem>
        _Rect<Elem> apply(const _Rect<Elem> &rect) const noexcept{
            return {
                rect.x + left,
                rect.y + top,
                rect.w - right - left,
                rect.h - bottom - top
            };
        }
        template<class Elem>
        _Margin<Elem> cast() const noexcept{
            return {
                static_cast<Elem>(left),
                static_cast<Elem>(top),
                static_cast<Elem>(right),
                static_cast<Elem>(bottom)
            };
        }
    };

    using Margin = _Margin<int>;
    using FMargin = _Margin<float>;
    //Margin --end

    //Bezier curve
    template<class T>
    struct _BezierCurve{
        using value_type = T;
        using point_type = _Point<T>;
        /**
         * @brief Constructor
         * 
         */
        _BezierCurve() noexcept = default;
        /**
         * @brief Constructor
         * 
         * @param p1 
         * @param p2 
         * @param p3 
         * @param p4 
         */
        _BezierCurve(const _Point<T> &p1,const _Point<T> &p2,const _Point<T> &p3,const _Point<T> &p4) noexcept{
            this->p1 = p1;
            this->p2 = p2;
            this->p3 = p3;
            this->p4 = p4;
        }
        /**
         * @brief Construct from a line
         * 
         */
        template<class Elem>
        _BezierCurve(const _Line<Elem> &line) noexcept{
            p1 = line.p1();
            p2 = line.p2();
            p3 = p2;
            p4 = p1;
        }
        /**
         * @brief Construct from another bezier curve with different elem
         * 
         */
        template<class Elem>
        _BezierCurve(const _BezierCurve<Elem> &curve) noexcept{
            p1 = curve.p1;
            p2 = curve.p2;
            p3 = curve.p3;
            p4 = curve.p4;
        }

        /**
         * @brief Copy constructor
         * 
         */
        _BezierCurve(const _BezierCurve &) noexcept = default;
        ~_BezierCurve() noexcept = default;
        /**
         * @brief Cast template
         * 
         */
        template<class Elem>
        _BezierCurve<Elem> cast() const noexcept{
            return {
                p1.template cast<Elem>(),
                p2.template cast<Elem>(),
                p3.template cast<Elem>(),
                p4.template cast<Elem>()
            };
        }
        /**
         * @brief Compare
         * 
         */
        bool compare(const _BezierCurve &curve) const noexcept{
            return p1 == curve.p1 and p2 == curve.p2 and p3 == curve.p3 and p4 == curve.p4;
        }
        /**
         * @brief Operator for compare
         * 
         */
        bool operator ==(const _BezierCurve &curve) const noexcept{
            return compare(curve);
        }
        /**
         * @brief Operator for compare
         * 
         */
        bool operator !=(const _BezierCurve &curve) const noexcept{
            return not compare(curve);
        }

        _Point<T> p1;//< begin point
        _Point<T> p2;//< control point 1
        _Point<T> p3;//< control point 2
        _Point<T> p4;//< end point
    };

    using BezierCurve = _BezierCurve<float>;
    using FBezierCurve = _BezierCurve<float>;

    //Paths

    //Polygen --begin
    /**
     * @brief Polygen
     * @note I think i use github codepilot to generate this code
     * @tparam T 
     */
    template<class T>
    struct _Polygen{
        using value_type = T;
        //Make iterator alias from vector
        using iterator = typename std::vector<_Point<T>>::iterator;
        using const_iterator = typename std::vector<_Point<T>>::const_iterator;

        std::vector<_Point<T>> points;
        
        //Construct
        _Polygen() noexcept = default;
        _Polygen(const _Polygen &) noexcept = default;
        _Polygen(std::initializer_list<_Point<T>> points) noexcept{
            this->points = points;
        }
        /**
         * @brief Construct from a rectangle
         * 
         * @param points 
         * @return template<class Elem> 
         */
        template<class Elem>
        _Polygen(const _Rect<Elem> &points) noexcept{
            this->points = {
                {points.x,points.y},
                {points.x + points.w,points.y},
                {points.x + points.w,points.y + points.h},
                {points.x,points.y + points.h},
            };
        }
        /**
         * @brief Construct from a bounds
         * 
         * @param points 
         * @return template<class Elem> 
         */
        template<class Elem>
        _Polygen(const _Bounds<Elem> &points) noexcept{
            this->points = {
                {points.minx,points.miny},
                {points.maxx,points.miny},
                {points.maxx,points.maxy},
                {points.minx,points.maxy},
            };
        }

        //Autocast
        template<class Elem>
        _Polygen(std::initializer_list<_Point<Elem>> points) noexcept{
            for(auto &p : points){
                this->points.push_back({p.x,p.y});
            }
        }
        //Method
        _Point<T> &operator [](size_t index){
            return points[index];
        }
        const _Point<T> &operator [](size_t index) const{
            return points[index];
        }
        size_t size() const noexcept{
            return points.size();
        }
        bool  empty() const noexcept{
            return points.empty();
        }
        void  clear(){
            points.clear();
        }
        /**
         * @brief Is the invalid
         * 
         * @param p 
         * @return true 
         * @return false 
         */
        bool invalid() const noexcept{
            return points.size() < 3;
        }

        //Compare
        bool compare(const _Polygen &p) const noexcept{
            if(size() != p.size()){
                return false;
            }
            for(size_t i = 0;i < size();++i){
                if(points[i] != p[i]){
                    return false;
                }
            }
            return true;
        }
        //Iterator from vector
        iterator begin(){
            return points.begin();
        }
        const_iterator begin() const{
            return points.begin();
        }
        iterator end(){
            return points.end();
        }
        const_iterator end() const{
            return points.end();
        }

        /**
         * @brief Add point
         * 
         */
        void push_back(const _Point<T> &p){
            points.push_back(p);
        }
        /**
         * @brief Remove point
         * 
         */
        void pop_back(){
            points.pop_back();
        }
        /**
         * @brief Add point
         * 
         */
        void add_point(const _Point<T> &p){
            points.push_back(p);
        }
        void add_point(T x,T y){
            points.push_back({x,y});
        }


        /**
         * @brief Get the bounds
         * 
         */
        _Bounds<T> bounds() const noexcept{
            if(empty()){
                return {};
            }
            _Bounds<T> b;
            b.minx = points[0].x;
            b.miny = points[0].y;
            b.maxx = points[0].x;
            b.maxy = points[0].y;
            for(size_t i = 1;i < size();++i){
                b.minx = min(b.minx,points[i].x);
                b.miny = min(b.miny,points[i].y);
                b.maxx = max(b.maxx,points[i].x);
                b.maxy = max(b.maxy,points[i].y);
            }
            return b;
        }

        //Helper
        bool operator ==(const _Polygen &poly) const noexcept{
            return compare(poly);
        }
        bool operator !=(const _Polygen &poly) const noexcept{
            return not compare(poly);
        }
        //Cast
        template<class Elem>
        _Polygen<Elem> cast() const{
            return *this;
        }
        /**
         * @brief Construct a new Rect from bounds
         * 
         * @tparam Elem 
         */
        template<class Elem>
        operator _Polygen<Elem>() const noexcept{
            _Polygen<Elem> poly;
            poly.points.resize(size());
            for(size_t i = 0;i < size();++i){
                poly.points[i] = {
                    static_cast<Elem>(points[i].x),
                    static_cast<Elem>(points[i].y)
                };
            }
            return poly;
        }
    };

    using Polygen = _Polygen<int>;
    using FPolygen = _Polygen<float>;
    //Polygen --end

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
    using MarginImpl = _Margin<T>;
    template<class T>
    using BoundsImpl = _Bounds<T>;
    template<class T>
    using PolygenImpl = _Polygen<T>;
    template<class T>
    using BezierCurveImpl = _BezierCurve<T>;

    //Utils
    template<class T1,class T2,class T3>
    inline bool PointInCircle(const _Point<T1> &center,T2 r,const _Point<T3> &point) noexcept{
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
    //TODO List
    //PointInShape
    //LineInShape

    /**
     * @brief Point inside polygen
     * 
     */
    template<class T>
    inline bool PointInPolygen(const _Polygen<T> &poly,const _Point<T> &point) noexcept{
        if(poly.size() < 3)
            return false;
        bool inside = false;
        for(size_t i = 0,j = poly.size() - 1;i < poly.size();j = i++){
            if(
                (
                    (
                        (
                            poly[i].y > point.y
                        ) != (
                            poly[j].y > point.y
                        )
                    )
                    and
                    (
                        point.x < (
                            poly[j].x - poly[i].x
                        ) * (
                            point.y - poly[i].y
                        ) / (
                            poly[j].y - poly[i].y
                        ) + poly[i].x
                    )
                )
                or
                (
                    poly[i].x == poly[j].x
                    and
                    point.x == poly[i].x
                )
            )
                inside = not inside;
        }
        return inside;
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

        bool d1 = _segCrossSeg_(p1,p2,leftTop,rightTop);
        bool d2 = _segCrossSeg_(p1,p2,rightTop,rightDown);
        bool d3 = _segCrossSeg_(p1,p2,rightDown,leftDown);
        bool d4 = _segCrossSeg_(p1,p2,leftDown,leftTop);

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
    inline T IntersectRect(const T &r1,const T &r2) noexcept{
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
     * @brief Get union of two rects
     * 
     * @param num 
     * @return template<class T> 
     */
    template<class T>
    inline T UnionRect(const T &r1,const T &r2) noexcept{
        auto minx = min(r1.x,r2.x);
        auto miny = min(r1.y,r2.y);
        auto maxx = max(r1.x + r1.w,r2.x + r2.w);
        auto maxy = max(r1.y + r1.h,r2.y + r2.h);
        return T(
            minx,
            miny,
            max(static_cast<decltype(minx)>(0),maxx - minx),
            max(static_cast<decltype(minx)>(0),maxy - miny)
        );
    }
    //float number utils
    template<class T>
    inline size_t GetFloatPrecision(T num) noexcept{
        //Get decimal fraction
        T a = num - std::floor(num);

        size_t n = 0;

        while(a != std::floor(a)){
            n += 1;
            a *= 10;
        }
        return n;
    }
    template<class T>
    inline T SetFloatPrecision(T num,size_t n) noexcept{
        size_t prec = GetFloatPrecision(num);
        if(prec <= n){
            //do nothing
            return num;
        }
        //TODO
        T result = std::floor(num);
        T dec = num - result;

        for(size_t i = 0;i < n;i++){

        }
        return 0;
    }
    //TODO Shape Utils

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
    BTKAPI std::ostream &operator <<(std::ostream&,const FMargin &);
}
#endif // _BTK_RECT_HPP_
