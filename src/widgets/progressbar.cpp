#include "../build.hpp"

#include <Btk/progressbar.hpp>
#include <Btk/render.hpp>
#include <Btk/themes.hpp>

#include <algorithm>

namespace Btk{
    ProgressBar::ProgressBar(){
        set_value(0);
    }
    ProgressBar::~ProgressBar() = default;

    void ProgressBar::draw(Renderer &render){
        FRect fixed_rect = rect;

        fixed_rect.y += 1;
        fixed_rect.x += 1;
        fixed_rect.w -= 1;
        fixed_rect.h -= 1;

        render.begin_path();
        render.rect(fixed_rect);
        render.fill_color(theme().active.button);
        render.fill();

        render.begin_path();
        render.rect(fixed_rect);
        render.stroke_color(theme().active.border);
        render.stroke();



        //Draw the next boarder
        if(value != 0){
            render.begin_path();
            FRect r = fixed_rect;
            r.w = r.w / 100.0f * value;
            render.rect(r);
            render.fill_color(theme().active.highlight);
            render.fill();
        }
        
        //Draw text
        if(display_value){
            render.begin_path();
            render.use_font(font()); 
            render.text_align(TextAlign::Middle | TextAlign::Center);
            render.fill_color(theme().active.text);
            render.text(fixed_rect.center(),value_text);
            render.fill();
        }
    
    }
    void ProgressBar::set_value(float v){
        v = std::clamp(v,0.0f,100.0f);
        value_text = u8format(fmt_string.c_str(),v);
        value = v;
        _signal_value_changed(v);
        redraw();
    }
}