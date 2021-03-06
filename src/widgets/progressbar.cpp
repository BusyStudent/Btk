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
        fixed_rect.w -= 1;
        fixed_rect.h -= 1;

        render.begin_path();
        render.rect(rect);
        render.fill_color(theme()[Theme::Button]);
        render.fill();

        render.begin_path();
        render.rect(rect);
        render.stroke_color(theme()[Theme::Border]);
        render.stroke();



        //Draw the next boarder
        render.begin_path();
        FRect r = rect;
        r.w = r.w / 100.0f * value;
        render.rect(r);
        render.fill_color(theme()[Theme::Highlight]);
        render.fill();
        
        //Draw text
        if(display_value){
            render.begin_path();
            render.text_align(TextAlign::Middle | TextAlign::Center);
            render.text_size(theme().font_size()); 
            render.fill_color(theme()[Theme::Text]);
        }
    
        render.text(rect.center(),value_text);
        render.fill();
    }
    void ProgressBar::set_value(float v){
        v = std::clamp(v,0.0f,100.0f);
        value_text = u8format("%.2f%%",v);
        value = v;
        _signal_value_changed(v);
        redraw();
    }
}