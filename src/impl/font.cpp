#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "../build.hpp"

#include <Btk/impl/font.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/platform.hpp>
#include <Btk/pixels.hpp>
#include <Btk/font.hpp>
#include <Btk/rect.hpp>

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
        Font f = FromFile(pimpl->filename,pimpl->ptsize);
        //First open font
        //Set style
        TTF_SetFontStyle(f.pimpl->font,TTF_GetFontStyle(pimpl->font));
        return f;
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
    //UNICODE Versions
    PixBuf Font::render_solid(std::u16string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUNICODE_Solid(
            pimpl->font,reinterpret_cast<const Uint16*>(text.data()),
            color
        );
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    PixBuf Font::render_shaded(std::u16string_view text,Color fg,Color bg){
        SDL_Surface *surf = TTF_RenderUNICODE_Shaded(
            pimpl->font,
            reinterpret_cast<const Uint16*>(text.data()),
            fg,
            bg
        );
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    PixBuf Font::render_blended(std::u16string_view text,Color color){
        SDL_Surface *surf = TTF_RenderUNICODE_Blended(
            pimpl->font,
            reinterpret_cast<const Uint16*>(text.data()),
            color
        );
        if(surf == nullptr){
            throwSDLError(TTF_GetError());
        }
        return surf;
    }
    //font style
    FontStyle Font::style() const {
        return static_cast<FontStyle>(TTF_GetFontStyle(pimpl->font));
    }
    std::string Font::style_name() const{
        char *name = TTF_FontFaceStyleName(pimpl->font);
        if(name == nullptr){
            throwSDLError(TTF_GetError());
        }
        std::string s(name);
        SDL_free(name);
        return s;
    }
    std::string Font::family() const{
        char *name = TTF_FontFaceFamilyName(pimpl->font);
        if(name == nullptr){
            throwSDLError(TTF_GetError());
        }
        std::string s(name);
        SDL_free(name);
        return s;
    }
    //size
    Size Font::size(std::string_view text){
        int w,h;
        if(TTF_SizeUTF8(pimpl->font,text.data(),&w,&h) != 0){
            w = -1;
            h = -1;
        }
        return {w,h};
    }
    Size Font::size(std::u16string_view text){
        int w,h;
        if(TTF_SizeUNICODE(pimpl->font,
            reinterpret_cast<const Uint16*>(text.data()),
            &w,
            &h) != 0){
                
            w = -1;
            h = -1;
        }
        return {w,h};
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