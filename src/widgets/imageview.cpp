#include <SDL2/SDL.h>

#include <Btk/impl/widget.hpp>
#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/imageview.hpp>
namespace Btk{
    struct ImageViewImpl:public WidgetImpl{
        ImageViewImpl(WindowImpl *win,SDL_Rect pos);
        ~ImageViewImpl();
        //texture of image
        void draw();
        SDL_Texture *texture;
    };
    //Init the imageciew
    ImageViewImpl::ImageViewImpl(WindowImpl *win,SDL_Rect pos){
        this->win = win;
        this->rect = pos;
    }
    ImageViewImpl::~ImageViewImpl(){
        //Destroy the texture
        if(texture != nullptr){
            SDL_DestroyTexture(texture);
        }
    }
    void ImageViewImpl::draw(){
        SDL_Renderer *render = win->render->render;
        //...
        SDL_RenderCopy(render,texture,nullptr,&rect);
    }
};
namespace Btk{
    ImageView::ImageView(Window &win,int x,int y,int w,int h){

    }
    
};