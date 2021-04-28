#include "../build.hpp"

#include <Btk/themes.hpp>
#include <ostream>

namespace Btk{
    Palette::Palette() = default;
    Palette::Palette(const Palette &) = default;
    Palette::Palette(Palette &&) = default;
    Palette::~Palette() = default;

    Palette &Palette::operator =(const Palette &) = default;
    Palette &Palette::operator =(Palette &&) = default;
}
namespace Btk{
    void Palette::set(std::string_view key,Color c){
        colors[std::string(key)] = c;
    }
    Color Palette::get(std::string_view key) const{
        return colors.at(std::string(key));
    }
    bool Palette::has_color(std::string_view key) const{
        return colors.find(std::string(key)) != colors.end();
    }
    size_t Palette::size() const{
        return colors.size();
    }
    std::ostream &operator <<(std::ostream &os,const Palette &p){
        os << '}' << '\n';
        for(auto &i:p.colors){
            os << '\t' <<i.first << ':' << i.second << '\n';
        }
        os << '}' << '\n';
        return os;
    }
}