#include <SDL2/SDL.h>

#include "../build.hpp"

#include <Btk/impl/core.hpp>
#include <Btk/exception.hpp>
#include <Btk/platform.hpp>
#include <Btk/pixels.hpp>
#include <Btk/font.hpp>
#include <Btk/rect.hpp>

#include <thread>
#include <mutex>

#include "../libs/fontstash.h"

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
    Font::Font(const Font &f){
        font = BtkFt_Dup(f.font);
        ptsize_ = f.ptsize_;
    }
    //TODO Improve Cache proformance
    Font::Font(u8string_view fontname,int ptsize){
        font = nullptr;
        open(fontname,ptsize);
    }
    //Open font by name
    void Font::open(u8string_view fontname,int ptsize){
        close();
        openfile(find_font(fontname),ptsize);
    }
    //Open font by file
    void Font::openfile(u8string_view filename,int ptsize){
        auto new_font = BtkFt_Open(filename.data(),0);
        close();
        font = new_font;
        ptsize_ = ptsize;
    }
    Font Font::FromFile(u8string_view filename,int ptsize){
        auto *font = BtkFt_Open(filename.data(),0);
        return Font(font,ptsize);
    }
    Font::Font(void *f,float ptsize){
        ptsize_ = ptsize;
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
        BtkFt_Close(font);
    }
    Font Font::clone() const{
        return Font(BtkFt_Dup(font),ptsize_);
    }

    // PixBuf Font::render_solid(u8string_view text,Color color){
    //     // SDL_Surface *surf = TTF_RenderUTF8_Solid(pimpl->font,text.data(),color);
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");

    // }
    // PixBuf Font::render_shaded(u8string_view text,Color fg,Color bg){
    //     // SDL_Surface *surf = TTF_RenderUTF8_Shaded(
    //     //     pimpl->font,
    //     //     text.data(),
    //     //     fg,
    //     //     bg
    //     // );
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");
    // }
    // PixBuf Font::render_blended(u8string_view text,Color color){
    //     // SDL_Surface *surf = TTF_RenderUTF8_Blended(pimpl->font,text.data(),color);
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");
    // }
    // //UNICODE Versions
    // PixBuf Font::render_solid(u16string_view text,Color color){
    //     // SDL_Surface *surf = TTF_RenderUNICODE_Solid(
    //     //     pimpl->font,reinterpret_cast<const Uint16*>(text.data()),
    //     //     color
    //     // );
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");
    // }
    // PixBuf Font::render_shaded(u16string_view text,Color fg,Color bg){
    //     // SDL_Surface *surf = TTF_RenderUNICODE_Shaded(
    //     //     pimpl->font,
    //     //     reinterpret_cast<const Uint16*>(text.data()),
    //     //     fg,
    //     //     bg
    //     // );
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");
    // }
    // PixBuf Font::render_blended(u16string_view text,Color color){
    //     // SDL_Surface *surf = TTF_RenderUNICODE_Blended(
    //     //     pimpl->font,
    //     //     reinterpret_cast<const Uint16*>(text.data()),
    //     //     color
    //     // );
    //     // if(surf == nullptr){
    //     //     throwSDLError(TTF_GetError());
    //     // }
    //     // return surf;
    //     throwRuntimeError("UnImpl yet");
    // }
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
            font = BtkFt_Dup(f.font);
        }
        return *this;
    }
    Font &Font::operator =(Font &&f){
        if(&f != this){
            close();
            font = f.font;
            f.font = nullptr;
        }
        return *this;
    }
    int Font::refcount() const noexcept{
        // return font->refcount;
        return BtkFt_Refcount(font);
    }
    void Font::Init(){
        BtkFt_Init();
    }
    void Font::Quit(){
        BtkFt_Quit();
    }
};