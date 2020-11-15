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

namespace Btk{
    //Font
    FontImpl::FontImpl(std::string_view fname,int size){
        
        refcount = 1;
        
        font = TTF_OpenFont(fname.data(),size);
        if(font == nullptr){
            throwSDLError(TTF_GetError());
        }
        ptsize = size;
        filename = fname;
    }
    FontImpl::~FontImpl(){
        TTF_CloseFont(font);
    }
    //Ref
    FontImpl *FontImpl::ref(){
        ++refcount;
        return this;
    }
    void FontImpl::unref(){
        --refcount;
        if(refcount <= 0){
            //No object hold it
            delete this;
        }
    }
}
namespace Btk{
    Font::~Font(){
        close();
    }
    //Copt font
    Font::Font(const Font &font){
        if(font.pimpl != nullptr){
            pimpl = font.pimpl->ref();
        }
        else{
            pimpl = nullptr;
        }
    }
    Font::Font(std::string_view fontname,int ptsize){
        pimpl = nullptr;
        pimpl = new FontImpl(
            FontUtils::GetFileByName(fontname),
            ptsize
        );
    }
    //Open font by name
    void Font::open(std::string_view fontname,int ptsize){
        openfile(
            FontUtils::GetFileByName(fontname),ptsize
        );
    }
    //Open font by file
    void Font::openfile(std::string_view filename,int ptsize){
        FontImpl *new_font = new FontImpl(
            filename,ptsize
        );
        close();
        pimpl = new_font;
    }
    Font Font::FromFile(std::string_view filename,int ptsize){
        return Font(
            new FontImpl(filename,ptsize)
        );
    }

    int Font::ptsize() const noexcept{
        return pimpl->ptsize;
    }
    void Font::set_ptsize(int new_ptsize){
        FontImpl *new_font = new FontImpl(
            pimpl->filename,
            new_ptsize
        );
        close();
        pimpl = new_font;
    }
    void Font::close(){
        if(pimpl != nullptr){
            pimpl->unref();
            pimpl = nullptr;
        }
    }
    Font Font::clone() const{
        return FromFile(pimpl->filename,pimpl->ptsize);
    }

    PixBuf Font::render_solid(std::string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUTF8_Solid(pimpl->font,text.data(),color);
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    PixBuf Font::render_shaded(std::string_view text,Color fg,Color bg){
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
    PixBuf Font::render_blended(std::string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUTF8_Blended(pimpl->font,text.data(),color);
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    Font &Font::operator =(const Font &f){
        if(&f != this){
            close();
            if(f.pimpl != nullptr){
                pimpl = f.pimpl->ref();
            }
        }
        return *this;
    }
    Font &Font::operator =(Font &&f){
        if(&f != this){
            close();
            pimpl = f.pimpl;
            f.pimpl = nullptr;
        }
        return *this;
    }
};