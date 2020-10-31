#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#ifdef BTK_USE_GFX
    #include "../thirdparty/SDL2_rotozoom.h"
#endif

#include <Btk/exception.hpp>
#include <Btk/pixels.hpp>
#include <Btk/rwops.hpp>
namespace Btk{
    Surface::~Surface(){
        SDL_FreeSurface(surf);
    }
    Surface::Surface(std::string_view file){
        surf = IMG_Load(file.data());
        if(surf == nullptr){
            throwSDLError(IMG_GetError());
        }
    }
    Surface Surface::ref() const{
        surf->refcount ++;
        return Surface(surf);
    }
    Surface Surface::clone() const{
        SDL_Surface *surf = SDL_DuplicateSurface(this->surf);
        if(surf == nullptr){
            throwSDLError(SDL_GetError());
        }
        else{
            return Surface(surf);
        }
    }
    //save surface
    void Surface::save_bmp(RWops &rw){
        if(SDL_SaveBMP_RW(surf,rw.get(),false) == -1){
            throwSDLError();
        }
    }
    void Surface::save_jpg(RWops &rw,int quality){
        if(IMG_SaveJPG_RW(surf,rw.get(),false,quality) == -1){
            throwSDLError();
        }
    }
    void Surface::save_png(RWops &rw,int quality){
        if(IMG_SaveJPG_RW(surf,rw.get(),false,quality) == -1){
            throwSDLError();
        }
    }

    void Surface::save_bmp(std::string_view fname){
        auto rw = RWops::FromFile(fname.data(),"rb");
        Surface::save_bmp(rw);
    }
    void Surface::save_jpg(std::string_view fname,int quality){
        auto rw = RWops::FromFile(fname.data(),"rb");
        Surface::save_jpg(rw,quality);
    }
    void Surface::save_png(std::string_view fname,int quality){
        auto rw = RWops::FromFile(fname.data(),"rb");
        Surface::save_png(rw,quality);
    }
    //operators
    Surface &Surface::operator =(SDL_Surface *sf){
        SDL_FreeSurface(surf);
        surf = sf;
    }
    Surface &Surface::operator =(Surface &&sf){
        SDL_FreeSurface(surf);
        surf = sf.surf;
        sf.surf = nullptr;
    }
    //static method
    Surface Surface::FromMem(const void *mem,size_t size){
        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromConstMem(mem,size),true);
        if(surf == nullptr){
            throwSDLError(IMG_GetError());
        }
        return Surface(surf);
    }
    Surface Surface::FromFile(std::string_view file){
        return Surface(file);
    }
    Surface Surface::FromFile(FILE *f){
        SDL_Surface *surf = IMG_Load_RW(SDL_RWFromFP(f,SDL_FALSE),true);
        if(surf == nullptr){
            throwSDLError(IMG_GetError());
        }
        return Surface(surf);
    }
    Surface Surface::FromXPMArray(char **da){
        SDL_Surface *surf = IMG_ReadXPMFromArray(da);
        if(surf == nullptr){
            throwSDLError(IMG_GetError());
        }
        return Surface(surf);
    }
    Surface Surface::FromRWops(RWops &rwops){
        SDL_Surface *surf = IMG_Load_RW(rwops.get(),false);
        if(surf == nullptr){
            throwSDLError();
        }
        return Surface(surf);
    }
    //Textures
    Texture::~Texture(){
        SDL_DestroyTexture(texture);
    }
    Texture &Texture::operator =(SDL_Texture *t){
        SDL_DestroyTexture(texture);
        texture = t;
        return *this;
    }
    Texture &Texture::operator =(Texture &&t){
        SDL_DestroyTexture(texture);
        texture = t.texture;
        t.texture = nullptr;
        return *this;
    }
};