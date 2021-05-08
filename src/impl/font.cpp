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

#include "../font/internal.hpp"

//Cast to the implment

#define FONTIMPL(X) reinterpret_cast<BtkFt::Font*>(X)

namespace Btk{
    static BtkFt::Font *OpenFont(std::string_view fname,float ptsize){
        BtkFt::Init();
        auto *face = BtkFt::Instance().add_font(fname.data(),0);
        return new BtkFt::Font(face,ptsize);
    }
    static BtkFt::Font *OpenFontBy(std::string_view fontname,float ptsize){
        BtkFt::Init();

        #if 1
        if(fontname.find('/') != fontname.npos){
            //Is filename
            return OpenFont(fontname,ptsize);
        }
        #endif

        auto &ins = BtkFt::Instance();
        auto *face = ins.find_font(fontname);
        if(face == nullptr){
            return OpenFont(FontUtils::GetFileByName(fontname),ptsize);
        }
        return new BtkFt::Font(face,ptsize);
    }
}
namespace Btk{
    Font::~Font(){
        close();
    }
    //Copt font
    Font::Font(const Font &font){
        if(font.pimpl != nullptr){
            pimpl = font.pimpl;
            FONTIMPL(pimpl)->ref();
        }
        else{
            pimpl = nullptr;
        }
    }
    Font::Font(std::string_view fontname,int ptsize){
        pimpl = nullptr;
        pimpl = OpenFontBy(fontname,ptsize);
    }
    //Open font by name
    void Font::open(std::string_view fontname,int ptsize){
        auto *new_font = OpenFontBy(fontname,ptsize);
        close();
        pimpl = new_font;
    }
    //Open font by file
    void Font::openfile(std::string_view filename,int ptsize){
        auto *new_font = OpenFont(filename,ptsize);
        close();
        pimpl = new_font;
    }
    Font Font::FromFile(std::string_view filename,int ptsize){
        return Font(
            OpenFont(filename,ptsize)
        );
    }



    bool Font::has_glyph(char32_t ch) const{
        //return TTF_GlyphIsProvided(pimpl->font,ch);
        return FONTIMPL(pimpl)->has_glyph(ch);
    }
    float Font::height() const{
        //return TTF_FontHeight(pimpl->font);
        return FONTIMPL(pimpl)->height();
    }
    int Font::kerning_size(char32_t prev,char32_t cur) const{
        //return TTF_GetFontKerningSizeGlyphs(pimpl->font,prev,cur);
        return FONTIMPL(pimpl)->kerning_size(prev,cur);
    }

    int Font::ptsize() const noexcept{
        return FONTIMPL(pimpl)->ptsize;
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
            FONTIMPL(pimpl)->unref();
            pimpl = nullptr;
        }
    }
    Font Font::clone() const{
        if(pimpl == nullptr){
            return Font();
        }
        return Font(new BtkFt::Font(*FONTIMPL(pimpl)));
    }

    PixBuf Font::render_solid(std::string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUTF8_Solid(pimpl->font,text.data(),color);
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");

    }
    PixBuf Font::render_shaded(std::string_view text,Color fg,Color bg){
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
    PixBuf Font::render_blended(std::string_view text,Color color){
        // SDL_Surface *surf = TTF_RenderUTF8_Blended(pimpl->font,text.data(),color);
        // if(surf == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // return surf;
        throwRuntimeError("UnImpl yet");
    }
    //UNICODE Versions
    PixBuf Font::render_solid(std::u16string_view text,Color color){
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
    PixBuf Font::render_shaded(std::u16string_view text,Color fg,Color bg){
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
    PixBuf Font::render_blended(std::u16string_view text,Color color){
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
    std::string Font::style_name() const{
        // char *name = TTF_FontFaceStyleName(pimpl->font);
        // if(name == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // std::string s(name);
        // SDL_free(name);
        // return s;
        return FONTIMPL(pimpl)->style_name();
    }
    std::string Font::family() const{
        // char *name = TTF_FontFaceFamilyName(pimpl->font);
        // if(name == nullptr){
        //     throwSDLError(TTF_GetError());
        // }
        // std::string s(name);
        // SDL_free(name);
        // return s;
        return FONTIMPL(pimpl)->family_name();
    }
    //size
    Size Font::size(std::string_view text){
        // int w,h;
        // if(TTF_SizeUTF8(pimpl->font,text.data(),&w,&h) != 0){
        //     w = -1;
        //     h = -1;
        // }
        // return {w,h};
        return FONTIMPL(pimpl)->text_size(text);
    }
    Size Font::size(std::u16string_view text){
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
                FONTIMPL(pimpl)->ref();
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
        return FONTIMPL(pimpl)->refcount;
    }
};