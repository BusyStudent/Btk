#include <SDL2/SDL.h>

#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
#include <Btk/Btk.hpp>
namespace Btk{
    ImageView::ImageView() = default;
    ImageView::ImageView(int x,int y,int w,int h){
        rect.x = x;
        rect.y = y;
        rect.w = w;
        rect.h = h;
        //User defined its position
        attr.user_rect = true;
    }
    ImageView::~ImageView(){

    }
    void ImageView::draw(Renderer &render){
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
            //render image
            FRect dst = rect;
            FRect src = image_rect;
            render.draw_image(texture,&src,&dst);
            //render.restore();
        }
        if(draw_borader){
            render.begin_path();
            render.rect(rect);
            render.stroke_color(boarder_color);
            render.stroke();
        }
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
};