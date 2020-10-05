#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#ifdef BTK_USE_GFX
    #include <SDL2/SDL2_rotozoom.h>
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
};