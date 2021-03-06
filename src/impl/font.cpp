#include <SDL2/SDL.h>

#include "../build.hpp"

#include <Btk/font/system.hpp>
#include <Btk/font/font.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/platform.hpp>
#include <Btk/pixels.hpp>
#include <Btk/font.hpp>
#include <Btk/rect.hpp>

#include <thread>
#include <mutex>


//Cast to the implment

namespace Btk{
    static u8string find_font(u8string_view name){
        if(name.find('/')  != name.npos or 
           name.find('\\') != name.npos){

            //Is filename
            return name;
        }
        else{
            return FontUtils::GetFileByName(name);
        }
    }
    Font::~Font(){
        close();
    }
    //Copt font
    Font::Font(const Font &font){
        if(font.pimpl != nullptr){
            pimpl = font.pimpl;
            pimpl->ref();
        }
        else{
            pimpl = nullptr;
        }
    }
    //TODO Improve Cache proformance
    Font::Font(u8string_view fontname,int ptsize){
        pimpl = nullptr;
        open(fontname,ptsize);
    }
    //Open font by name
    void Font::open(u8string_view fontname,int ptsize){
        auto *new_font = Ft::GlobalCache().load_font(fontname,0);
        close();
        pimpl = new_font;
        pimpl->set_ptsize(ptsize);
    }
    //Open font by file
    void Font::openfile(u8string_view filename,int ptsize){
        auto *new_font = new Ft::Font(filename.data(),0);
        close();
        pimpl = new_font;
        pimpl->set_ptsize(ptsize);
    }
    Font Font::FromFile(u8string_view filename,int ptsize){
        auto *font = new Ft::Font(filename.data(),0);
        font->set_ptsize(ptsize);
        return font;
    }



    bool Font::has_glyph(char32_t ch) const{
        //return TTF_GlyphIsProvided(pimpl->font,ch);
        // return FONTIMPL(pimpl)->has_glyph(ch);
        throwRuntimeError("Unimpl yet");
    }
    float Font::height() const{
        //return TTF_FontHeight(pimpl->font);
        // return FONTIMPL(pimpl)->height();
        throwRuntimeError("Unimpl yet");
    }
    int Font::kerning_size(char32_t prev,char32_t cur) const{
        //return TTF_GetFontKerningSizeGlyphs(pimpl->font,prev,cur);
        // return FONTIMPL(pimpl)->kerning_size(prev,cur);
        throwRuntimeError("Unimpl yet");
    }

    int Font::ptsize() const noexcept{
        // return FONTIMPL(pimpl)->ptsize;
        throwRuntimeError("Unimpl yet");
    }
    void Font::set_ptsize(int new_ptsize){
        // FontImpl *new_font = new FontImpl(
        //     pimpl->filename,
        //     new_ptsize
        // );
        // close();
        // pimpl = new_font;
        throwRuntimeError("UnImpl yet");
    }
    void Font::close(){
        if(pimpl != nullptr){
            pimpl->unref();
            pimpl = nullptr;
        }
    }
    Font Font::clone() const{
        if(pimpl == nullptr){
            return Font();
        }
        return Font(new Ft::Font(*pimpl));
    }

    PixBuf Font::render_solid(u8string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUTF8_Solid(pimpl->font,text.data(),color);
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");

    }
    PixBuf Font::render_shaded(u8string_view text,Color fg,Color bg){
        // SDL_Surface *surf = TTF_RenderUTF8_Shaded(
        //     pimpl->font,
        //     text.data(),
        //     fg,
        //     bg
        // );
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    PixBuf Font::render_blended(u8string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUTF8_Blended(pimpl->font,text.data(),color);
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    //UNICODE Versions
    PixBuf Font::render_solid(u16string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUNICODE_Solid(
        //     pimpl->font,reinterpret_cast<const Uint16*>(text.data()),
        //     color
        // );
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    PixBuf Font::render_shaded(u16string_view text,Color fg,Color bg){
        // SDL_Surface *surf = TTF_RenderUNICODE_Shaded(
        //     pimpl->font,
        //     reinterpret_cast<const Uint16*>(text.data()),
        //     fg,
        //     bg
        // );
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    PixBuf Font::render_blended(u16string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUNICODE_Blended(
        //     pimpl->font,
        //     reinterpret_cast<const Uint16*>(text.data()),
        //     color
        // );
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    //font style
    FontStyle Font::style() const {
        // return static_cast<FontStyle>(TTF_GetFontStyle(pimpl->font));
        throwRuntimeError("UnImpl yet");
    }
    u8string Font::style_name() const{
        // char *name = TTF_FontFaceStyleName(pimpl->font);
        // if(name == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // std::string s(name);
        // SDL_free(name);
        // return s;
        // return FONTIMPL(pimpl)->style_name();
        throwRuntimeError("Unimpl yet");
    }
    u8string Font::family() const{
        // char *name = TTF_FontFaceFamilyName(pimpl->font);
        // if(name == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // std::string s(name);
        // SDL_free(name);
        // return s;
        // return FONTIMPL(pimpl)->family_name();
        throwRuntimeError("Unimpl yet");
    }
    //size
    Size Font::size(u8string_view text){
        // int w,h;
        // if(TTF_SizeUTF8(pimpl->font,text.data(),&w,&h) != 0){
        //     w = -1;
        //     h = -1;
        // }
        // return {w,h};
        // return FONTIMPL(pimpl)->text_size(text.base());
        throwRuntimeError("Unimpl yet");
    }
    Size Font::size(u16string_view text){
        // int w,h;
        // if(TTF_SizeUNICODE(pimpl->font,
        //     reinterpret_cast<const Uint16*>(text.data()),
        //     &w,
        //     &h) != 0){
                
        //     w = -1;
        //     h = -1;
        // }
        // return {w,h};
        throwRuntimeError("UnImpl yet");
    }
    Font &Font::operator =(const Font &f){
        if(&f != this){
            close();
            if(f.pimpl != nullptr){
                pimpl = f.pimpl;
                pimpl->ref();
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
    int Font::refcount() const noexcept{
        return pimpl->refcount;
    }
};