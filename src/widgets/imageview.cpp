#include <SDL2/SDL.h>

#include "../build.hpp"

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
namespace Btk{
    ImageView::ImageView(Container& w){
        parent = &w;
        image_rect.x = 0;
        image_rect.y = 0;
        image_rect.w = 0;
        image_rect.h = 0;

        rect.x = 0;
        rect.y = 0;
        rect.w = 0;
        rect.h = 0;

    }
    ImageView::ImageView(Container&wi,int x,int y,int w,int h){
        parent = &wi;

        image_rect.x = 0;
        image_rect.y = 0;
        image_rect.w = 0;
        image_rect.h = 0;

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
        if(not pixelbuf.empty()){
            if(texture.empty()){
                //create texture
                texture = render.create_from(pixelbuf);
            }
            //render image
            render.copy(texture,&image_rect,&rect);
        }
    }
    void ImageView::set_image(const PixBuf &buf){
        pixelbuf = buf.clone();
        texture = nullptr;
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        redraw();
    }
    void ImageView::ref_image(PixBuf &buf){
        pixelbuf = buf.ref();
        texture = nullptr;
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        redraw();
    }
    void ImageView::set_clip(const Rect &r){
        image_rect = r;
        redraw();
    }
};