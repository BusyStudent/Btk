#include <SDL2/SDL.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/imageview.hpp>
#include <Btk/window.hpp>
#include <Btk/pixels.hpp>
namespace Btk{
    ImageView::ImageView(){
        win = Window::Current;
        image_rect.x = 0;
        image_rect.y = 0;
        image_rect.w = 0;
        image_rect.h = 0;

        pos.x = 0;
        pos.y = 0;
        pos.w = 0;
        pos.h = 0;

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
            render.copy(texture,&image_rect,&pos);
        }
    }
    void ImageView::draw(){
        if(win != nullptr){
            win->draw();
        }
    }
    void ImageView::set_image(const PixBuf &buf){
        pixelbuf = buf.clone();
        texture = nullptr;
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        draw();
    }
    void ImageView::ref_image(PixBuf &buf){
        pixelbuf = buf.ref();
        texture = nullptr;
        //Set image rect pos
        image_rect.w = buf->w;
        image_rect.h = buf->h;
        draw();
    }
    void ImageView::set_clip(const Rect &r){
        image_rect = r;
        draw();
    }
};