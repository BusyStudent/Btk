#include <SDL2/SDL.h>

#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/imageview.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    //TODO AntiAlias

    ImageView::ImageView() = default;
    ImageView::ImageView(int x,int y,int w,int h):ImageView(){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        //User defined its position
    }
    ImageView::~ImageView() = default;
    void ImageView::draw(Renderer &render,Uint32){
        render.save();
        render.set_antialias(false);
        if(dirty){
            //Cleanup
            texture = nullptr;
            dirty = false;
        }
        if(draw_background){
            render.draw_box(rect,bg_color);
        }
        if(not pixelbuf.empty()){
            //render.save();
            if(texture.empty()){
                //create texture
                texture = render.create_from(pixelbuf,tex_flags);
            }
            // if(scale_fact != 1.0f){
            //     render.save();
            //     render.scale(scale_fact,scale_fact);
            // }
            //render image
            FRect dst = rect;
            Rect src = image_rect;
            render.draw_image(texture,&src,&dst);
            //render.restore();
            // if(scale_fact != 1.0f){
            //     render.restore();
            // }
        }
        if(draw_borader){
            render.begin_path();
            render.rect(rect);
            render.stroke_color(boarder_color);
            render.stroke();
        }
        render.restore();
    }
    void ImageView::set_image(const PixBuf &buf){
        pixelbuf = buf.clone();
        if(window() != nullptr and IsMainThread()){
            //We can create texture right now
            texture = renderer()->create_from(pixelbuf);
        }
        else{
            dirty = true;//< We should cleanup
        }
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        redraw();
    }
    void ImageView::set_image(PixBuf &&buf){
        pixelbuf = std::move(buf);
        if(window() != nullptr and IsMainThread()){
            //We can create texture right now
            texture = renderer()->create_from(pixelbuf);
        }
        else{
            dirty = true;//< We should cleanup
        }
        //Set image rect pos
        image_rect.w = pixelbuf->w;
        image_rect.h = pixelbuf->h;
        redraw();
    }
    void ImageView::ref_image(PixBuf &buf){
        pixelbuf = buf.ref();
        if(window() != nullptr and IsMainThread()){
            //We can create texture right now
            texture = renderer()->create_from(pixelbuf);
        }
        else{
            dirty = true;//< We should cleanup
        }
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        redraw();
    }
    void ImageView::set_draw_boarder(bool f){
        draw_borader = f;
        redraw();
    }
    void ImageView::set_clip(const Rect &r){
        image_rect = r;
        redraw();
    }
    bool ImageView::handle_drop(DropEvent &event){
        if(accept_drop and event.type() == DropEvent::DropFile){
            set_image(PixBuf::FromFile(event.text));
            return event.accept();
        }
        return false;
    }
    bool ImageView::handle_drag(DragEvent &event){
        if(not dragable){
            return event.reject();
        }
        // switch(event.type()){
        //     case Event::DragBegin:{
        //         break;
        //     }
        //     case Event::DragEnd:{
        //         break;
        //     }
        //     case Event::Drag:{
        //         image_rect.x += event.xrel;
        //         image_rect.y += event.yrel;

        //         image_rect.w += event.xrel;
        //         image_rect.h += event.yrel;
                
        //         redraw();
        //         BTK_LOGINFO("image_rect = {%d,%d,%d,%d}",BTK_UNPACK_RECT(image_rect));
        //         break;
        //     }
        //     default:{}
        // }
        return event.accept();
    }
    bool ImageView::handle_wheel(WheelEvent &event){
        // if(event.y > 0){
        //     scale_fact *= 2;
        // }
        // else{
        //     scale_fact /= 2;
        // }
        // redraw();
        return event.accept();
    }
};