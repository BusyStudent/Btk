#include "../build.hpp"

#include <Btk/graphics/base.hpp>
#include <Btk/exception.hpp>
#include <Btk/regex.hpp>

#ifdef BTK_USE_BUILTIN_GRADIENTS
    #include "./gradients_mk.inl"
#endif

namespace Btk{
    Gradient::Gradient(const Gradient &) = default;
    Gradient::Gradient(Gradient &&) = default;
    Gradient::~Gradient() = default;

    Gradient::Gradient(Preset v){
        _type = Linear;
        #ifdef BTK_USE_BUILTIN_GRADIENTS
        mk_gradient(*this,v);
        #else
        throwRuntimeError("No build with this support");
        #endif
    }

    Brush ParseBrush(u8string_view txt){
        if(txt.starts_with("LinearGradient(") && txt.ends_with(")")){
            //Like LinearGradient(Stop(0.0f,#ff0000),Stop(1.0f,#00ff00))
            //Stop(ANY,ANY)
            LinearGradient grad;
            Regex re(R"(Stop\(.+?\))");
            for(auto &stop : re.match(txt)){
                //Get the stop
                stop = stop.trim().substr(4,stop.length() - 5);
                //Ok now we have the float,Color
                size_t mark = stop.find(",");
                if(mark == u8string::npos){
                    throwRuntimeError("Invalid stop");
                }

                auto pos_str = stop.substr(0,mark);
                auto color_str = stop.substr(mark + 1);

                if(pos_str.back() == 'F' or pos_str.back() == 'f'){
                    pos_str.pop_back();
                }
                
                float pos = std::stof(pos_str);
                Color col = ParseColor(color_str);

                grad.add_color(pos,col);
            }
            return Brush(std::move(grad));
        }
        else{
            return ParseColor(txt);
        }
    }
}