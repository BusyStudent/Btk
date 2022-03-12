#include "../build.hpp"

#include <Btk/container.hpp>
#include <Btk/render.hpp>

#include <Btk/event.hpp>
namespace Btk{
    GroupBox::GroupBox(){
        background_color = theme().active.background;
        borader_color = theme().active.border;
    }
    GroupBox::~GroupBox() = default;
    void GroupBox::draw(Renderer &render){
        if(draw_background){
            render.draw_box(rectangle(),background_color);
        }
        if(draw_boarder){
            render.draw_rect(rectangle(),borader_color);
        }
    }
}