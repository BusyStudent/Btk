#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <Btk/impl/font.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/platform.hpp>
#include <Btk/pixels.hpp>
#include <Btk/font.hpp>

#include <thread>
#include <mutex>

#ifdef BTK_USE_FONTCONFIG
    #include <fontconfig/fontconfig.h>
#endif
namespace Btk{
    //Font
    FontImpl::FontImpl(std::string_view fname,int size){
        //Init Font System
        Platform::InitFont();
        
        font  = TTF_OpenFont(fname.data(),size);
        if(font == nullptr){
            throwSDLError(TTF_GetError());
        }
        ptsize = size;
        filename = fname;
        refcount = 1;
    }
    FontImpl::~FontImpl(){
        TTF_CloseFont(font);
    }
    
}
namespace Btk{
    Font::~Font(){
        delete pimpl;
    }
    //Open font
    Font Font::FromFile(std::string_view filename,int ptsize){
        return Font(
            new FontImpl(filename,ptsize)
        );
    }

    int Font::ptsize() const noexcept{
        return pimpl->ptsize;
    }
    Surface Font::render_solid(std::string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUTF8_Solid(pimpl->font,text.data(),color);
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    Surface Font::render_shaded(std::string_view text,Color fg,Color bg){
        SDL_Surface *surf = TTF_RenderUTF8_Shaded(
            pimpl->font,
            text.data(),
            fg,
            bg
        );
        
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    Surface Font::render_blended(std::string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUTF8_Blended(pimpl->font,text.data(),color);
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
};