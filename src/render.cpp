#include "./build.hpp"

#include <Btk/detail/scope.hpp>
#include <Btk/detail/utils.hpp>
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

#define BTK_CHECK_TEXTURE(TEX) \
    if(TEX.render != this){ \
        Btk::throwRendererError("Texture from different renderer");\
    }

namespace Btk{
    Texture Renderer::load(u8string_view fname,TextureFlags flags){
        return create_from(PixBuf::FromFile(fname),flags);
    }
    Texture Renderer::load(RWops &rwops,TextureFlags flags){
        return create_from(PixBuf::FromRWops(rwops),flags);
    }
    void Renderer::_draw_image(TextureID texture,float x,float y,float w,float h,float angle,float alpha){
        //make pattern

        auto paint = nvgImagePattern(
            nvg_ctxt,
            x,
            y,
            w,
            h,
            angle,
            texture,
            alpha
        );
        nvgBeginPath(nvg_ctxt);
        nvgRect(nvg_ctxt,x,y,w,h);
        nvgFillPaint(nvg_ctxt,paint);
        nvgFill(nvg_ctxt);
        
    }
    void Renderer::_draw_image(TextureID texture,const FRect *src,const FRect *dst){

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
            return _draw_image(texture,_dst.x,_dst.y,_dst.w,_dst.h);
        }
        if(_dst.empty() or src->empty()){
            return;
        }
        auto [tex_w,tex_h] = device()->texture_size(nvg_ctxt,texture);

        if(src->w == tex_w and src->h == tex_h){
            //We donnot need clipping
            return _draw_image(texture,_dst.x,_dst.y,_dst.w,_dst.h);
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
            texture,
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
    void Renderer::draw_image(PixBufRef pixbuf,float x,float y,float w,float h,float angle){
        CachedItem *cache = find_cache(w,h);
        if(cache == nullptr){
            //No cache useable
            auto texture = create_from(pixbuf);
            draw_image(texture,x,y,w,h,angle);
            //Add it to cache
            CachedItem item;
            item.w = texture.w();
            item.h = texture.h();
            item.tex = texture.detach();
            item.used = true;
            cached_texs.push_back(item);
            return;
        }
        //TODO need improve
        cache->used = true;
        update_texture(cache->tex,{0,0,int(w),int(h)},pixbuf->pixels);
        //Send a draw request
        auto paint = nvgImagePattern(
            nvg_ctxt,
            x,
            y,
            w,
            h,
            angle,
            cache->tex,
            1.0f
        );
        nvgBeginPath(nvg_ctxt);
        nvgRect(nvg_ctxt,x,y,w,h);
        nvgFillPaint(nvg_ctxt,paint);
        nvgFill(nvg_ctxt);
    }
    void Renderer::draw_image(PixBufRef pixbuf,const FRect *src,const FRect *dst){
        auto texture = create_from(pixbuf);
        draw_image(texture,src,dst);
        //Add it to cache
        CachedItem item;
        item.w = texture.w();
        item.h = texture.h();
        item.tex = texture.detach();
        item.used = true;
        cached_texs.push_back(item);
    }
    auto Renderer::find_cache(int w,int h) -> CachedItem *{
        for(auto &item:cached_texs){
            if(not item.used and item.w >= w and item.h >= h){
                return &item;
            }
        }
        return nullptr;
    }
}
namespace Btk{
    //NVG Method
    void Renderer::set_alpha(float alpha){
        nvgGlobalAlpha(nvg_ctxt,alpha);
    }
    void Renderer::set_pathwinding(PathWinding v){
        nvgPathWinding(nvg_ctxt,int(v));
    }
    void Renderer::set_linejoin(LineJoin c){
        nvgLineJoin(nvg_ctxt,int(c));
    }
    void Renderer::set_linecap(LineCap c){
        nvgLineCap(nvg_ctxt,int(c));
    }
    void Renderer::set_miterlimit(float limit){
        nvgMiterLimit(nvg_ctxt,limit);
    }
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
    //TextBox
    void Renderer::textbox(float x,float y,float width,u8string_view _text){
        std::string_view text = _text.base();
        #ifdef _MSC_VER
        nvgTextBox(nvg_ctxt,x,y,width,&*text.begin(),&*text.end());
        #else
        nvgTextBox(nvg_ctxt,x,y,width,text.begin(),text.end());
        #endif
    }
    //Sizeof
    FBounds Renderer::text_bounds(float x,float y,u8string_view s){
        float bounds[4];
        nvgTextBounds(nvg_ctxt,x,y,&s.front(),&s.back(),bounds);
        BTK_LOGINFO("bounds = {%f,%f,%f,%f}",
            bounds[0],
            bounds[1],
            bounds[2],
            bounds[3]
        );
        return {
            bounds[0],
            bounds[1],
            bounds[2],
            bounds[3]
        };
    }
    FBounds Renderer::textbox_bounds(float x,float y,float w,u8string_view s){
        float bounds[4];
        nvgTextBoxBounds(nvg_ctxt,x,y,w,&s.front(),&s.back(),bounds);
        BTK_LOGINFO("bounds = {%f,%f,%f,%f}",
            bounds[0],
            bounds[1],
            bounds[2],
            bounds[3]
        );
        return {
            bounds[0],
            bounds[1],
            bounds[2],
            bounds[3]
        };
    }
    //Get the font height
    float Renderer::font_height(){
        float lineh;
        nvgTextMetrics(nvg_ctxt,nullptr,nullptr,&lineh);
        return lineh;
    }
    //Get the rendered text size
    FSize Renderer::measure_text(u8string_view _view){
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
    //Set font ptsize
    void Renderer::text_size(float ptsize){
        nvgFontSize(nvg_ctxt,ptsize);
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
    void Renderer::triangle(float x1,float y1,float x2,float y2,float x3,float y3){
        nvgMoveTo(nvg_ctxt,x1,y1);
        nvgLineTo(nvg_ctxt,x2,y2);
        nvgLineTo(nvg_ctxt,x3,y3);
        nvgClosePath(nvg_ctxt);
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
    //Target
    void Renderer::set_target(Texture &target){
        if(target.empty()){
            throwRuntimeError("empty texture");
        }
        device()->set_target(nvg_ctxt,target.get());
    }
    void Renderer::reset_target(){
        device()->reset_target(nvg_ctxt);
    }
    void Renderer::set_antialias(bool val){
        nvgShapeAntiAlias(nvg_ctxt,val);
    }
    //Transform
    void Renderer::scale(float x_factor,float y_factor){
        nvgScale(nvg_ctxt,x_factor,y_factor);
    }
    void Renderer::skew_x(float angle){
        nvgSkewX(nvg_ctxt,angle);
    }
    void Renderer::skew_y(float angle){
        nvgSkewY(nvg_ctxt,angle);
    }
    void Renderer::rotate(float angle){
        nvgRotate(nvg_ctxt,angle);
    }

    void Renderer::translate(float x,float y){
        nvgTranslate(nvg_ctxt,x,y);
    }
    void Renderer::reset_transform(){
        nvgResetTransform(nvg_ctxt);
    }
}
namespace Btk{
    Renderer::Renderer(RendererDevice &dev,bool val){
        _device = &dev;
        free_device = val;

        nvg_ctxt = device()->create_context();
    }
    Renderer::~Renderer(){
        //Cleanup buffer
        for(auto iter = cached_texs.begin();iter != cached_texs.end();){
            device()->destroy_texture(nvg_ctxt,iter->tex);
            iter = cached_texs.erase(iter);
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
    Texture Renderer::create_from(PixBufRef buf,TextureFlags f){
        TextureID id = device()->create_texture_from(nvg_ctxt,buf,f);
        return Texture(id,this);
    }

    void Renderer::begin(){
        mtx.lock();
        auto [w,h] = device()->logical_size();
        //Init viewport
        device()->set_viewport(nullptr);
        //Begin frame
        begin_frame(w,h,float(device()->physical_size().w) / float(w));
    }
    void Renderer::end(){
        if(is_drawing){
            device()->end_frame(nvg_ctxt);
            device()->swap_buffer();
            //Too many caches
            if(cached_texs.size() > max_caches){
                int n = cached_texs.size() - max_caches;
                BTK_LOGINFO("[Renderer]Clear %d textures",n);
                for(int i = 0;i < n;i++){
                    device()->destroy_texture(nvg_ctxt,cached_texs.front().tex);
                    cached_texs.pop_front();
                }
                //Set another caches to unused
                for(auto &item:cached_texs){
                    item.used = false;
                }
            }
            is_drawing = false;
            //Free the queue
            while(not free_texs.empty()){
                destroy_texture(free_texs.front());
                free_texs.pop();
            }
            mtx.unlock();
        }
    }
}
namespace Btk{
    Size TextureRef::size() const{
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
        texture = -1;
        render = nullptr;        
    }
    void Texture::defer_clear(){
        if(empty()){
            return;
        }
        std::lock_guard locker(render->mtx);
        BTK_LOGINFO("[Texture] defer clear %d for %p",texture,render);
        render->free_texs.push(texture);
        
        texture = -1;
        render = nullptr;
    }

    //Update...

    void TextureRef::update(const void *pixels){
        if(empty() or pixels == nullptr){
            return;
        }
        render->update_texture(texture,pixels);
    }
    void TextureRef::update(PixBufRef pixbuf){
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
    void TextureRef::update(const Rect &rect,const void *pixels){
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
    void TextureRef::update(const Rect *rect,const void *pixels,Uint32 fmt){
        auto dev  = render->device();
        auto ctxt = render->context();
        Rect r;
        if(rect == nullptr){

        }
    }
    void TextureRef::native_handle(void *p_handle){
        if(empty()){
            throwRuntimeError("empty texture");
        }
        render->get_texture_handle(texture,p_handle);
    }
    Texture TextureRef::clone() const{
        if(empty()){
            throwRuntimeError("empty texture");
        }
        return Texture(render->clone_texture(texture),render);
    }
    PixBuf  TextureRef::dump() const{
        if(empty()){
            throwRuntimeError("empty texture");
        }
        //OK Try to get pixels from lock_texture
        auto [w,h] = size();
        auto ctxt = render->context();
        auto dev  = render->device();
        void *pix = nullptr;
        
        pix = dev->lock_texture(ctxt,texture,nullptr,LockFlag::Read);
        if(pix == nullptr){
            //Lock failed
            throwRuntimeError(dev->get_error());
        }
        PixBuf pixbuf(w,h,PixelFormat::RGBA32);
        memcpy(pixbuf->pixels,pix,w * h * SDL_BYTESPERPIXEL(PixelFormat::RGBA32));
        dev->unlock_texture(ctxt,texture,pix);
        return pixbuf;
    }
    TextureFlags TextureRef::flags() const{
        if(empty()){
            throwRuntimeError("empty texture");
        }
        return render->get_texture_flags(texture);
    }
    void TextureRef::set_flags(TextureFlags flags){
        if(empty()){
            throwRuntimeError("empty texture");
        }
        if(not render->set_texture_flags(texture,flags)){
            throwRendererError(render->device()->get_error());
        }
    }
    Texture &Texture::operator =(Texture &&t){
        if(&t == this){
            return *this;
        }
        clear();
        render   = t.render;
        texture  = t.texture;

        t.render = nullptr;
        t.texture = -1;
        return *this;
    }

    void Brush::clear(){
        if(_type == BrushType::Image){
            data.image.buf.~PixBuf();
        }
        _type = BrushType::Color;
        data.color = Color(0,0,0,0);
    }
    Brush::Brush(const Brush &brush):Brush(){
        if(brush._type != BrushType::Image){
            //Just memcpy
            memcpy(this,&brush,sizeof(Brush));
            return;
        }
        new (&(data.image.buf)) PixBuf(brush.data.image.buf);
        data.image.flags = brush.data.image.flags;
        _type = BrushType::Image;
    }
    void Brush::assign(const Brush &brush){
        clear();
        if(brush._type != BrushType::Image){
            //Just memcpy
            memcpy(this,&brush,sizeof(Brush));
            return;
        }
        new (&(data.image.buf)) PixBuf(brush.data.image.buf);
        data.image.flags = brush.data.image.flags;
        _type = BrushType::Image;
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
namespace Btk{
    //Draw functions
    void Renderer::use_brush(const Brush &brush){
        cur_brush = brush;
        switch(brush.type()){
            case BrushType::Color:{
                fill_color(brush.color());
                break;
            }
            default:{
                BTK_UNIMPLEMENTED();
            }
        }            
    }
    void Renderer::draw_rounded_box(const FRect &r,float rad,Color c){
        begin_path();
        fill_color(c);
        rounded_rect(r,rad);
        fill();
    }
    void Renderer::draw_rounded_rect(const FRect &r,float rad,Color c){
        begin_path();
        stroke_color(c);
        rounded_rect(r,rad);
        stroke();
    }
    void Renderer::draw_line(float x,float y,float x2,float y2,Color c){
        begin_path();
        stroke_color(c);
        move_to(x,y);
        line_to(x2,y2);
        stroke();
    }
    void Renderer::draw_rect(const FRect &r,Color c){
        begin_path();
        stroke_color(c);
        rect(r);
        stroke();
    }
    void Renderer::draw_box(const FRect &r,Color c){
        begin_path();
        fill_color(c);
        rect(r);
        fill();
    }
    void Renderer::draw_circle(float x,float y,float rad,Color c){
        begin_path();
        stroke_color(c);
        circle(x,y,rad);
        stroke();
    }
    void Renderer::fill_circle(float x,float y,float rad,Color c){
        begin_path();
        fill_color(c);
        circle(x,y,rad);
        fill();
    }
    void Renderer::draw_ellipse(float x,float y,float rx,float ry,Color c){
        begin_path();
        stroke_color(c);
        ellipse(x,y,rx,ry);
        stroke();
    }
    void Renderer::fill_ellipse(float x,float y,float rx,float ry,Color c){
        begin_path();
        fill_color(c);
        ellipse(x,y,rx,ry);
        fill();
    }
    void Renderer::draw_text(float x,float y,u8string_view txt,Color c){
        begin_path();
        fill_color(c);
        text(x,y,txt);
        fill();
    }
}