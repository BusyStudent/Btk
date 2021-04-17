#include "../build.hpp"

#include <Btk/rect.hpp>
#include <ostream>

namespace Btk{
    std::ostream &operator <<(std::ostream &os,const Rect &r){
        os << '(' << r.x << ',' << r.y << ',' << r.w << ',' << r.h << ')';
        return os;
    }
    std::ostream &operator <<(std::ostream &os,const FRect &r){
        os << '(' << r.x << ',' << r.y << ',' << r.w << ',' << r.h << ')';
        return os;
    }
    std::ostream &operator <<(std::ostream &os,const Size &r){
        os << '(' << r.w << ',' << r.h << ')';
        return os;
    }
    std::ostream &operator <<(std::ostream &os,const FSize &r){
        os << '(' << r.w << ',' << r.h << ')';
        return os;
    }
    std::ostream &operator <<(std::ostream &os,const Vec2 &r){
        os << '(' << r.x << ',' << r.y << ')';
        return os;
    }
    std::ostream &operator <<(std::ostream &os,const FVec2 &r){
        os << '(' << r.x << ',' << r.y << ')';
        return os;
    }
}