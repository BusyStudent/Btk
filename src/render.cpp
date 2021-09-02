#include "./build.hpp"

#include <Btk/impl/scope.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/rwops.hpp>
#include <Btk/font.hpp>
#include <SDL2/SDL.h>

#include <algorithm>
#include <cstdarg>
#include <limits>
#include <vector>


extern "C"{
    #include "libs/fontstash.h"
    #include "libs/nanovg.h"
}

#define UNPACK_COLOR(C) C.r,C.g,C.b,C.a
#define BTK_NULLCHECK(VAL) if(VAL == nullptr){return;}

static_assert(sizeof(Btk::RendererPaint) == sizeof(NVGpaint));
static_assert(sizeof(Btk::GLColor) == sizeof(NVGcolor));

namespace Btk{
    void Renderer::end(){
        if(is_drawing){
            device()->end_frame(nvg_ctxt);
            device()->swap_buffer();
            //Too many caches
            if(t_caches.size() > max_caches){
                int n = t_caches.size() - max_caches;
                BTK_LOGINFO("Clear %d textures",n);
                for(int i = 0;i < n;i++){
                    device()->destroy_texture(nvg_ctxt,t_caches.front());
                    t_caches.pop_front();
                }
            }
            is_drawing = false;
        }
    }
    Texture Renderer::load(u8string_view fname,TextureFlags flags){
        return create_from(PixBuf::FromFile(fname),flags);
    }
    Texture Renderer::load(RWops &rwops,TextureFlags flags){
        return create_from(PixBuf::FromRWops(rwops),flags);
    }
    void Renderer::draw_image(const Texture &texture,float x,float y,float w,float h,float angle){
        //make pattern
        auto paint = nvgImagePattern(
            nvg_ctxt,
            x,
            y,
            w,
            h,
            angle,
            texture.get(),
            1.0f
        );
        nvgBeginPath(nvg_ctxt);
        nvgRect(nvg_ctxt,x,y,w,h);
        nvgFillPaint(nvg_ctxt,paint);
        nvgFill(nvg_ctxt);
        
    }
    void Renderer::draw_image(const Texture &texture,const FRect *src,const FRect *dst){
        FRect _dst;
        if(dst != nullptr){
            _dst = *dst;
        }
        else{
            //Get current screnn output size
            auto [w,h] = device()->logical_size();
            // SDL_GetWindowSize(window,&w,&h);
            _dst.x = 0;
            _dst.y = 0;
            _dst.w = w;
            _dst.h = h;
        }
        if(src == nullptr){
            //We donnot need clipping
            return draw_image(texture,_dst);
        }
        if(_dst.empty() or src->empty()){
            return;
        }
        auto [tex_w,tex_h] = texture.size();

        if(src->w == tex_w and src->h == tex_h){
            //We donnot need clipping
            return draw_image(texture,_dst);
        }

        FRect _src;
        float max_float = std::numeric_limits<float>::max();
        _src.x = std::clamp(src->x,0.0f,max_float);
        _src.y = std::clamp(src->y,0.0f,max_float);
        _src.w = std::clamp(src->w,0.0f,float(tex_w));
        _src.h = std::clamp(src->h,0.0f,float(tex_h));

        //Save the status
        //save();
        //nvgIntersectScissor(nvg_ctxt,_dst.x,_dst.y,_dst.w,_dst.h);

        float w_ratio = float(tex_w) / _src.w;
        float h_ratio = float(tex_h) / _src.h;

        FRect target;

        target.w = _dst.w * w_ratio;
        target.h = _dst.h * h_ratio;

        target.x = _dst.x - (src->x / src->w) * _dst.w;
        target.y = _dst.y - (src->y / src->h) * _dst.h;


        auto paint = nvgImagePattern(
            nvg_ctxt,
            target.x,
            target.y,
            target.w,
            target.h,
            0,
            texture.get(),
            1.0f
        );
        //nvgResetScissor(nvg_ctxt);
        nvgBeginPath(nvg_ctxt);
        nvgRect(nvg_ctxt,_dst.x,_dst.y,_dst.w,_dst.h);
        nvgFillPaint(nvg_ctxt,paint);
        nvgFill(nvg_ctxt);
        //restore();
    }
    //Temp copy
    void Renderer::draw_image(const PixBuf &pixbuf,float x,float y,float w,float h,float angle){
        auto texture = create_from(pixbuf);
        draw_image(texture,x,y,w,h,angle);
        t_caches.emplace_back(texture.detach());
    }
    void Renderer::draw_image(const PixBuf &pixbuf,const FRect *src,const FRect *dst){
        auto texture = create_from(pixbuf);
        draw_image(texture,src,dst);
        t_caches.emplace_back(texture.detach());
    }
}
namespace Btk{
    static int TranslateAlign(Align v_align,Align h_align){
        int val = 0;
        switch(h_align){
            case Align::Center:
                val |= NVG_ALIGN_MIDDLE;
                break;
            case Align::Top:
                val |= NVG_ALIGN_TOP;
                break;
            case Align::Bottom:
                val |= NVG_ALIGN_BOTTOM;
                break;
            case Align::Baseline:
                val |= NVG_ALIGN_BASELINE;
                break;
            default:
                throwRuntimeError("Invaid Align");
        }
        switch(v_align){
            case Align::Right:
                val |= NVG_ALIGN_RIGHT;
                break;
            case Align::Left:
                val |= NVG_ALIGN_LEFT;
                break;
            case Align::Center:
                val |= NVG_ALIGN_CENTER;
                break;
            default:
                throwRuntimeError("Invaid Align");
        }
        return val;
    }
    //NVG Method
    void Renderer::fill(){
        nvgFill(nvg_ctxt);
    }
    void Renderer::stroke(){
        nvgStroke(nvg_ctxt);
    }
    void Renderer::stroke_width(float size){
        nvgStrokeWidth(nvg_ctxt,size);
    }
    void Renderer::stroke_color(Color c){
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
    }
    void Renderer::stroke_color(const GLColor& c){
        nvgStrokeColor(nvg_ctxt,nvgRGBAf(UNPACK_COLOR(c)));
    }
    void Renderer::fill_color(Color c){
        nvgFillColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
    }
    void Renderer::fill_color(const GLColor &c){
        nvgFillColor(nvg_ctxt,nvgRGBAf(UNPACK_COLOR(c)));
    }
    void Renderer::show_path_caches(){
        nvgDebugDumpPathCache(nvg_ctxt);
    }
    void Renderer::move_to(float x,float y){
        nvgMoveTo(nvg_ctxt,x,y);
    }
    void Renderer::line_to(float x,float y){
        nvgLineTo(nvg_ctxt,x,y);
    }
    void Renderer::arc_to(float x1,float y1,float x2,float y2,float radius){
        nvgArcTo(nvg_ctxt,x1,y1,x2,y2,radius);
    }
    void Renderer::quad_to(float cx,float cy,float x,float y){
        nvgQuadTo(nvg_ctxt,cx,cy,x,y);
    }
    void Renderer::bezier_to(float c1x,float c1y,float c2x,float c2y,float x,float y){
        nvgBezierTo(nvg_ctxt,c1x,c1y,c2x,c2y,x,y);
    }
    void Renderer::begin_path(){
        nvgBeginPath(nvg_ctxt);
    }
    void Renderer::close_path(){
        nvgClosePath(nvg_ctxt);
    }
    //Text
    void Renderer::text(float x,float y,u8string_view _text){
        std::string_view text = _text.base();
        #ifdef _MSC_VER
        nvgText(nvg_ctxt,x,y,&*text.begin(),&*text.end());
        #else
        nvgText(nvg_ctxt,x,y,text.begin(),text.end());
        #endif
    }
    void Renderer::text(float x,float y,u16string_view _text){
        auto &buf = FillInternalU8Buffer(_text);
        
        text(x,y,buf);
    }
    //TextBox
    void Renderer::textbox(float x,float y,float width,u8string_view _text){
        std::string_view text = _text.base();
        #ifdef _MSC_VER
        nvgTextBox(nvg_ctxt,x,y,width,&*text.begin(),&*text.end());
        #else
        nvgTextBox(nvg_ctxt,x,y,width,text.begin(),text.end());
        #endif
    }
    void Renderer::textbox(float x,float y,float width,u16string_view text){
        auto &buf = FillInternalU8Buffer(text);

        nvgTextBox(nvg_ctxt,x,y,width,&buf[0],nullptr);
    }
    //Sizeof
    FSize Renderer::glyph_size(char16_t ch){
        auto &buf = FillInternalU8Buffer(std::u16string_view(&ch,1));
        NVGtextRow  row;
        nvgTextBreakLines(nvg_ctxt,&buf[0],nullptr,std::numeric_limits<float>::max(),&row,1);
        return FSize{
            (row.width),
            font_height()
            
        };
    }
    //Get the font height
    float Renderer::font_height(){
        float lineh;
        nvgTextMetrics(nvg_ctxt,nullptr,nullptr,&lineh);
        return lineh;
    }
    //Get the rendered text size
    FSize Renderer::text_size(u16string_view view){
        auto &buf = FillInternalU8Buffer(view);
        NVGtextRow  row;
        nvgTextBreakLinesEx(nvg_ctxt,&buf[0],nullptr,std::numeric_limits<float>::max(),&row,1);
        return FSize{
            row.width,
            font_height()
        };
    }
    FSize Renderer::text_size(u8string_view _view){
        auto view = _view.base();
        NVGtextRow  row;
        nvgTextBreakLinesEx(nvg_ctxt,&view.front(),&view.back() + 1,std::numeric_limits<float>::max(),&row,1);
        return FSize{
            row.width,
            font_height()
        };
    }
    void Renderer::text_align(TextAlign align){
        nvgTextAlign(nvg_ctxt,int(align));
    }
    void Renderer::text_align(Align v_align,Align h_align){
        nvgTextAlign(nvg_ctxt,TranslateAlign(v_align,h_align));
    }
    //Set font ptsize
    void Renderer::text_size(float ptsize){
        nvgFontSize(nvg_ctxt,ptsize);
    }
    //Set font
    bool Renderer::use_font(u8string_view fontname) noexcept{
        auto &buf = FillInternalU8Buffer(fontname);
        int font_id = nvgFindFont(nvg_ctxt,buf.c_str());
        if(font_id == -1){
            return false;
        }
        nvgFontFaceId(nvg_ctxt,font_id);
        return true;
    }
    TextMetrics Renderer::font_metrics(){
        TextMetrics m;
        nvgTextMetrics(nvg_ctxt,&m.ascender,&m.descender,&m.h);
        return m;
    }
    //Path
    void Renderer::rect(float x,float y,float w,float h){
        nvgRect(nvg_ctxt,x,y,w,h);
    }
    void Renderer::rounded_rect(float x,float y,float w,float h,float rad){
        nvgRoundedRect(nvg_ctxt,x,y,w,h,rad);
    }
    void Renderer::circle(float center_x,float center_y,float r){
        nvgCircle(nvg_ctxt,center_x,center_y,r);
    }
    void Renderer::ellipse(float center_x,float center_y,float rx,float ry){
        nvgEllipse(nvg_ctxt,center_x,center_y,rx,ry);
    }
    //R/S
    void Renderer::save(){
        nvgSave(nvg_ctxt);
    }
    void Renderer::restore(){
        nvgRestore(nvg_ctxt);
    }
    //Scissor
    void Renderer::scissor(float x,float y,float w,float h){
        nvgScissor(nvg_ctxt,x,y,w,h);
    }
    void Renderer::intersest_scissor(float x,float y,float w,float h){
        nvgIntersectScissor(nvg_ctxt,x,y,w,h);
    }
    
    void Renderer::reset_scissor(){
        nvgResetScissor(nvg_ctxt);
    }
    //Begin or end Frame
    void Renderer::begin_frame(float w,float h,float ratio){
        if(not is_drawing){
            device()->begin_frame(nvg_ctxt,w,h,ratio);
            is_drawing = true;
        }
    }
    void Renderer::end_frame(){
        if(is_drawing){
            device()->end_frame(nvg_ctxt);
            is_drawing = false;
        }
    }
    bool Renderer::use_font(const Font &font){
        use_font(font.family());
        text_size(font.ptsize());
        return true;
    }
    void Renderer::set_antialias(bool val){
        nvgShapeAntiAlias(nvg_ctxt,val);
    }
    //Transform

    void Renderer::scale(float x_factor,float y_factor){
        nvgScale(nvg_ctxt,x_factor,y_factor);
    }
    void Renderer::translate(float x,float y){
        nvgTranslate(nvg_ctxt,x,y);
    }
}
namespace Btk{
    Renderer::Renderer(RendererDevice &dev,bool val){
        _device = &dev;
        free_device = val;

        nvg_ctxt = device()->create_context();
        //Add default font
        nvgFontFace(nvg_ctxt,"");
        nvgFontBlur(nvg_ctxt,0);
    }
    void Renderer::destroy(){
        if(device() == nullptr){
            return;
        }
        //Cleanup buffer
        for(auto iter = t_caches.begin();iter != t_caches.end();){
            device()->destroy_texture(nvg_ctxt,*iter);
            iter = t_caches.erase(iter);
        }
        device()->destroy_context(nvg_ctxt);

        if(free_device){
            delete device();
        }

        //Set pointer
        _device = nullptr;
        nvg_ctxt = nullptr;
    }
    Texture Renderer::create(int w,int h,TextureFlags f){
        TextureID id = device()->create_texture(nvg_ctxt,w,h,f);
        return Texture(id,this);
    }
    Texture Renderer::create_from(const PixBuf &buf,TextureFlags f){
        if(buf.empty()){
            return {};
        }
        if(buf->format->format != SDL_PIXELFORMAT_RGBA32){
            return create_from(buf.convert(SDL_PIXELFORMAT_RGBA32),f);
        }
        if(buf.must_lock()){
            buf.lock();
        }
        TextureID id = device()->create_texture(nvg_ctxt,buf->w,buf->h,f,buf->pixels);
        if(buf.must_lock()){
            buf.unlock();
        }
        return Texture(id,this);
    }

    void Renderer::begin(){
        auto [w,h] = device()->logical_size();
        //Init viewport
        device()->set_viewport(nullptr);
        //Begin frame
        begin_frame(w,h,float(device()->physical_size().w) / float(w));
    }
}
namespace Btk{
    Texture::~Texture(){
        clear();
    }
    Size Texture::size() const{
        Size size;
        nvgImageSize(render->nvg_ctxt,texture,&size.w,&size.h);
        return size;
    }
    //delete texture
    void Texture::clear(){
        if(empty()){
            return;
        }
        render->destroy_texture(texture);
        texture = 0;
        render = nullptr;        
    }

    //Update...

    void Texture::update(const void *pixels){
        if(empty() or pixels == nullptr){
            return;
        }
        render->update_texture(texture,pixels);
    }
    void Texture::update(const PixBuf &pixbuf){
        if(empty()){
            return;
        }
        if(pixbuf.empty()){
            throw RuntimeError("The pixbuf is empty");
        }
        if(pixbuf.size() != size()){
            throw RuntimeError("pixbuf.size() != size()");
        }
        //Check end
        if(pixbuf->format->format != SDL_PIXELFORMAT_RGBA32){
            //Convert it
            //Should we check lock here?
            render->update_texture(
                texture,
                pixbuf.convert(SDL_PIXELFORMAT_RGBA32)->pixels
            );
        }
        else{
            if(pixbuf.must_lock()){
                pixbuf.lock();
            }
            render->update_texture(texture,pixbuf->pixels);
            if(pixbuf.must_lock()){
                pixbuf.unlock();
            }
        }
    }
    void Texture::update(const Rect &rect,const void *pixels){
        if(empty()){
            throwRuntimeError("empty texture");
        }
        if(pixels == nullptr){
            return;
        }
        if(rect.empty()){
            throwRuntimeError("rect is empty");
        }
        auto s = size();
        if(rect.x + rect.w > s.w or rect.y + rect.h > s.h){
            throwRuntimeError("rect.size() > size()");
        }
        render->update_texture(texture,rect,pixels);
    }
    void Texture::native_handle(void *p_handle){
        if(empty()){
            throwRuntimeError("empty texture");
        }
        render->get_texture_handle(texture,p_handle);
    }
    Texture Texture::clone() const{
        if(empty()){
            throwRuntimeError("empty texture");
        }
        return Texture(render->clone_texture(texture),render);
    }
    // PixBuf  Texture::dump() const{
    //     if(empty()){
    //         throwRuntimeError("empty texture");
    //     }
    //     return render->dump_texture(texture);
    // }
    TextureFlags Texture::flags() const{
        if(empty()){
            throwRuntimeError("empty texture");
        }
        return render->get_texture_flags(texture);
    }
    // void Texture::set_flags(TextureFlags flags){
    //     if(empty()){
    //         throwRuntimeError("empty texture");
    //     }
    //     render->set_texture_flags(texture,flags);
    // }
    Texture &Texture::operator =(Texture &&t){
        if(&t == this){
            return *this;
        }
        clear();
        render   = t.render;
        texture  = t.texture;

        t.render = nullptr;
        t.texture = 0;
        return *this;
    }
}
namespace Btk{
    //Paint
    static RendererPaint from_nvgpaint(const NVGpaint &p){
        return reinterpret_cast<const RendererPaint&>(p);
    }
    void Renderer::stroke_paint(const RendererPaint &paint){
        nvgStrokePaint(nvg_ctxt,reinterpret_cast<const NVGpaint&>(paint));
    }
    void Renderer::fill_paint(const RendererPaint &paint){
        nvgFillPaint(nvg_ctxt,reinterpret_cast<const NVGpaint&>(paint));
    }
    RendererPaint Renderer::image_pattern(const FRect &r,float angle,TextureID tex,float alpha){
        return from_nvgpaint(
            nvgImagePattern(
                nvg_ctxt,
                r.x,
                r.y,
                r.w,
                r.h,
                angle,
                tex,
                alpha
            )
        );
    }
    RendererPaint Renderer::linear_gradient(float sx,float sy,float ex,float ey,GLColor in,GLColor out){
        return from_nvgpaint(
            nvgLinearGradient(
                nvg_ctxt,
                sx,
                sy,
                ex,
                ey,
                reinterpret_cast<const NVGcolor&>(in),
                reinterpret_cast<const NVGcolor&>(out)
            )
        );
    }
    RendererPaint Renderer::radial_gradient(float cx,float cy,float inr,float outr,GLColor in,GLColor out){
        return from_nvgpaint(
            nvgRadialGradient(
                nvg_ctxt,
                cx,
                cy,
                inr,
                outr,
                reinterpret_cast<const NVGcolor&>(in),
                reinterpret_cast<const NVGcolor&>(out)
            )
        );
    }
}
//Dreaptched functions
namespace Btk{
    //Draw a rounded box directly
    int Renderer::rounded_box(const Rect &r,int rad,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgRoundedRect(nvg_ctxt,r.x,r.y,r.w,r.h,rad);
        nvgFillColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgFill(nvg_ctxt);
        return 0;
    }
    int Renderer::rounded_rect(const Rect &r,int rad,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgRoundedRect(nvg_ctxt,r.x,r.y,r.w,r.h,rad);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
    int Renderer::box(const Rect &r,Color c){
        nvgBeginPath(nvg_ctxt);

        nvgRect(nvg_ctxt,r.x,r.y,r.w,r.h);
        nvgFillColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgFill(nvg_ctxt);
        return 0;
    }
    int Renderer::rect(const Rect &r,Color c){
        nvgBeginPath(nvg_ctxt);

        nvgRect(nvg_ctxt,r.x,r.y,r.w,r.h);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
    int  Renderer::line(int x,int y,int x2,int y2,Color c){
        nvgBeginPath(nvg_ctxt);
        nvgMoveTo(nvg_ctxt,x,y);
        nvgLineTo(nvg_ctxt,x2,y2);
        nvgStrokeColor(nvg_ctxt,nvgRGBA(UNPACK_COLOR(c)));
        nvgStroke(nvg_ctxt);
        return 0;
    }
}