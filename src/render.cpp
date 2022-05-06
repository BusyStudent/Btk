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
#include <set>


extern "C"{
    #include "libs/fontstash.h"
    #include "libs/nanovg.h"

    //Rect Pack
    #define STBRP_ASSERT BTK_ASSERT
    #define STB_RECT_PACK_IMPLEMENTATION
    #define STBRP_STATIC
    #include "libs/stb_rect_pack.h"
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
        auto r = text_bounds(0,0,_view).cast<FRect>();
        return {r.w,r.h}; 
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
    //Temp Texture for drawing
    struct Renderer::TempTexture{
        stbrp_node nodes[64];
        stbrp_context rp;
        void *pixels;//< Pixels buffer to update
        bool rp_init;
        int tex;
        int w,h;
        //Use this to avoid a object used multiple times in a single frame?
        // std::set<void*> object_hits;

        void init(NVGcontext *ctxt,int tex,int w,int h){
            this->w = w;
            this->h = h;
            this->tex = tex;
            this->rp_init = false;
            this->pixels = nullptr;
        }
        void clear(NVGcontext *ctxt){
            nvgDeleteImage(ctxt,tex);
            SDL_free(pixels);
        }
        /**
         * @brief If you want to pack,call this first
         * 
         */
        void begin_pack(){
            if(not rp_init){
                stbrp_init_target(&rp,w,h,nodes,SDL_arraysize(nodes));
                stbrp_setup_allow_out_of_mem(&rp,1);
                rp_init = true;
                //Lazy init pixels
                if(pixels == nullptr){
                    pixels = SDL_malloc(w * h * sizeof(Uint32));
                }
            }
        }
        /**
         * @brief Called at end()
         * 
         */
        void pack_cleanup(){
            rp_init = false;
        }
        /**
         * @brief Pack a rect
         * 
         * @param w 
         * @param h 
         * @param out 
         * @return true 
         * @return false 
         */
        bool pack(int w,int h,Point *out){
            stbrp_rect r;
            r.w = w;
            r.h = h;

            stbrp_pack_rects(&rp,&r,1);

            out->x = r.x;
            out->y = r.y;

            return r.was_packed;
        }
    };
    static FRect sub_image_pattern(int tex_w,int tex_h,const Rect &src,const FRect &dst){

        float w_ratio = float(tex_w) / float(src.w);
        float h_ratio = float(tex_h) / float(src.h);

        FRect target;

        target.w = dst.w * w_ratio;
        target.h = dst.h * h_ratio;

        target.x = dst.x - (src.x / src.w) * dst.w;
        target.y = dst.y - (src.y / src.h) * dst.h;

        return target;
    }

    Renderer::Renderer(RendererDevice &dev,bool val){
        _device = &dev;
        free_device = val;

        nvg_ctxt = device()->create_context();
    }
    Renderer::~Renderer(){
        //Cleanup buffer
        for(auto iter = temp_texs.begin();iter != temp_texs.end();){
            iter->clear(nvg_ctxt);
            iter = temp_texs.erase(iter);
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
            //Cleanup tmp tex
            flush_temp_tex();

            is_drawing = false;
            //Free the queue
            while(not free_texs.empty()){
                destroy_texture(free_texs.front());
                free_texs.pop();
            }
        }
    }

    //Temp texture
    void Renderer::flush_temp_tex(){
        if(temp_texs.size() > max_caches){
            //Cleanup
            //Sort by size
            std::sort(temp_texs.begin(),temp_texs.end(),
                [](const TempTexture &a,const TempTexture &b){
                    return a.w * a.h > b.w * b.h;
                }
            );
            //Cleanup with min size
            int n_to_clean = temp_texs.size() - max_caches;
            for(int i = 0;i < n_to_clean;i++){
                temp_texs.back().clear(nvg_ctxt);
                temp_texs.pop_back();
            }
        }
        //Reset pack state
        for(auto &tex : temp_texs){
            tex.pack_cleanup();
        }
    }
    auto Renderer::alloc_temp_tex(int w,int h,int *x,int *y) -> TempTexture *{
        Point where;
        for(auto &tex : temp_texs){
            tex.begin_pack();
            if(tex.pack(w,h,&where)){
                //Find position
                *x = where.x;
                *y = where.y;
                return &tex;
            }
        }
        int alloc_w = w * 2;
        int alloc_h = h * 2;
        //We need to create a new one
        int tex_id = device()->create_texture(nvg_ctxt,alloc_w,alloc_h,TextureFlags::Linear);
        if(tex_id == -1){
            throwRendererError(device()->get_error());
        }
        //add it
        temp_texs.push_front({});
        temp_texs.front().init(
            nvg_ctxt,
            tex_id,
            alloc_w,
            alloc_h
        );
        BTK_LOGINFO("[Renderer]alloc temp tex:%d,%d",alloc_w,alloc_h);
        BTK_LOGINFO("[Renderer]current temp tex:%d",temp_texs.size());
        //Pack again
        return alloc_temp_tex(w,h,x,y);
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
    void Renderer::_draw_image(TextureID texture,const Rect *src,const FRect *dst){

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

        Rect _src;
        int max_int = std::numeric_limits<int>::max();
        _src.x = std::clamp(src->x,0,max_int);
        _src.y = std::clamp(src->y,0,max_int);
        _src.w = std::clamp(src->w,0,tex_w);
        _src.h = std::clamp(src->h,0,tex_h);

        // Save the status
        // save();
        // nvgIntersectScissor(nvg_ctxt,_dst.x,_dst.y,_dst.w,_dst.h);

        // float w_ratio = float(tex_w) / float(_src.w);
        // float h_ratio = float(tex_h) / float(_src.h);

        // FRect target;

        // target.w = _dst.w * w_ratio;
        // target.h = _dst.h * h_ratio;

        // target.x = _dst.x - (src->x / src->w) * _dst.w;
        // target.y = _dst.y - (src->y / src->h) * _dst.h;

        auto target = sub_image_pattern(tex_w,tex_h,_src,_dst);

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
    void Renderer::draw_image(PixBufRef  buf,const Rect *_src,const FRect *_dst){
        if(buf.empty()){
            return;
        }
        //Get src if
        Rect src;
        if(_src == nullptr){
            src.x = 0;
            src.y = 0;
            src.w = buf->w;
            src.h = buf->h;
        }
        else{
            src = *_src;
            //ASSERT
            BTK_ASSERT(src.x >= 0 and src.y >= 0 and src.w >= 0 and src.h >= 0);
            BTK_ASSERT(src.w + src.x <= buf->w and src.h + src.y <= buf->h);
        }
        //Get dst if
        FRect dst;
        if(_dst == nullptr){
            auto [w,h] = device()->logical_size();
            dst.x = 0;
            dst.y = 0;
            dst.w = w;
            dst.h = h;
        }
        else{
            dst = *_dst;
        }
        if(dst.empty() or src.empty()){
            return;
        }

        //Alloc temp tex
        int x;
        int y;
        auto tex = alloc_temp_tex(src.w,src.h,&x,&y);

        uint8_t *buf_data = static_cast<uint8_t*>(buf->pixels) +
            src.y * buf->pitch +
            src.x * buf->format->BytesPerPixel;
        
        uint8_t *dst_data = reinterpret_cast<uint8_t*>(tex->pixels) +
            y * (tex->w * 4) +
            x * 4;

        //Update bitmap
        SDL_ConvertPixels(
            src.w,src.h,
            buf->format->format,
            buf_data,buf->pitch,
            SDL_PIXELFORMAT_RGBA32,
            dst_data,tex->w * 4
        );
        //Update texture
        Rect update_area = {
            x,y,src.w,src.h
        };
        update_texture(tex->tex,update_area,tex->pixels);
        //Draw
        _draw_image(tex->tex,&src,&dst);
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
        else if(_type == BrushType::Gradient){
            data.gradient.~Gradient();
        }
        _type = BrushType::Color;
        data.color = Color(0,0,0,0);
    }
    Brush::Brush(const Brush &brush):Brush(){
        assign(brush);
    }
    void Brush::assign(const Brush &brush){
        clear();
        if(brush._type == BrushType::Color){
            //Just memcpy
            memcpy(this,&brush,sizeof(Brush));
        }
        else if(brush._type == BrushType::Gradient){
            //Copy the gradient
            new(&data.gradient) Gradient(brush.data.gradient);
            _type = BrushType::Gradient;
        }
        else if(brush._type == BrushType::Image){
            new (&(data.image.buf)) PixBuf(brush.data.image.buf);
            data.image.flags = brush.data.image.flags;
            data.image.alpha = brush.data.image.alpha;
            _type = BrushType::Image;
        }
    }
    void Brush::assign(Brush &&brush){
        clear();
        if(brush._type == BrushType::Color){
            //Just memcpy
            memcpy(this,&brush,sizeof(Brush));
        }
        else if(brush._type == BrushType::Gradient){
            //Copy the gradient
            new(&data.gradient) Gradient(std::move(brush.data.gradient));
            _type = BrushType::Gradient;
        }
        else if(brush._type == BrushType::Image){
            new (&(data.image.buf)) PixBuf(std::move(brush.data.image.buf));
            data.image.flags = brush.data.image.flags;
            data.image.alpha = brush.data.image.alpha;
            _type = BrushType::Image;
        }
        //Reset prev types
        brush._type = BrushType::Color;
    }
    Brush::operator Color() const{
        if(type() != BrushType::Color){
            throwRuntimeError("Brush is not color");
        }
        return color();
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
    void Renderer::draw_rounded_box(const FRect &r,float rad,BrushRef c){
        begin_path();
        _apply_brush(c,r);
        rounded_rect(r,rad);
        fill();
    }
    void Renderer::draw_rounded_rect(const FRect &r,float rad,BrushRef c){
        begin_path();
        _apply_brush(c,r);
        rounded_rect(r,rad);
        stroke();
    }
    void Renderer::draw_line(float x,float y,float x2,float y2,BrushRef c){
        begin_path();
        //Calc then range
        float minx = min(x,x2);
        float miny = min(y,y2);
        float maxx = max(x,x2);
        float maxy = max(y,y2);

        FRect rect = {minx,miny,maxx - minx,maxy - miny};
        _apply_brush(c,rect);

        move_to(x,y);
        line_to(x2,y2);
        stroke();
    }
    void Renderer::draw_rect(const FRect &r,BrushRef c){
        begin_path();
        _apply_brush(c);
        rect(r);
        stroke();
    }
    void Renderer::draw_box(const FRect &r,BrushRef c){
        begin_path();
        _apply_brush(c,r);
        rect(r);
        fill();
    }
    void Renderer::draw_circle(float x,float y,float rad,BrushRef c){
        begin_path();
        //Calc the circle bounds
        FRect rect = {x-rad,y-rad,rad*2,rad*2};
        _apply_brush(c,rect);

        circle(x,y,rad);
        stroke();
    }
    void Renderer::fill_circle(float x,float y,float rad,BrushRef c){
        begin_path();
        //Calc the circle bounds
        FRect rect = {x-rad,y-rad,rad*2,rad*2};
        _apply_brush(c,rect);
        
        circle(x,y,rad);
        fill();
    }
    void Renderer::draw_ellipse(float x,float y,float rx,float ry,BrushRef c){
        begin_path();
        //Calc the ellipse bounds
        FRect rect = {x-rx,y-ry,rx*2,ry*2};
        _apply_brush(c,rect);

        ellipse(x,y,rx,ry);
        stroke();
    }
    void Renderer::fill_ellipse(float x,float y,float rx,float ry,BrushRef c){
        begin_path();
        //Calc the ellipse bounds
        FRect rect = {x-rx,y-ry,rx*2,ry*2};
        _apply_brush(c,rect);

        ellipse(x,y,rx,ry);
        fill();
    }
    void Renderer::draw_text(float x,float y,u8string_view txt,Color c){
        begin_path();
        fill_color(c);
        text(x,y,txt);
        fill();
    }

    //Brush Apply
    void Renderer::_apply_brush(BrushRef brush){
        BTK_ASSERT(brush.type() == BrushType::Color);
        stroke_color(brush.color());
    }
    void Renderer::_apply_brush(BrushRef brush,const FRect &r){
        switch(brush.type()){
            case BrushType::Color:{
                fill_color(brush.color());
                stroke_color(brush.color());
                break;
            }
            case BrushType::Gradient:{
                const Gradient &g = brush.graident();
                //Transform the gradient by coordinates
                FBounds grad_bds;

                if(g._mode == Gradient::Absolute){
                    grad_bds = g._area;
                }
                else{
                    //From 0 to 1
                    auto rel_rect = g._area.cast<FRect>();

                    grad_bds.minx = r.x + rel_rect.x * r.w;
                    grad_bds.miny = r.y + rel_rect.y * r.h;
                    grad_bds.maxx = r.x + rel_rect.w * r.w;
                    grad_bds.maxy = r.y + rel_rect.h * r.h;
                }

                if(g._type == Gradient::Linear){
                    //TODO : Simulate multi stop points by 2 points per stop
                    //Find min position and max position
                    auto &vec = g.colors;
                    Color min;
                    Color max;

                    float min_pos = 1.0f;
                    float max_pos = 0.0f;

                    //Find min and max
                    for(auto &c : vec){
                        if(c.position <= min_pos){
                            min = c.color;
                            min_pos = c.position;
                        }
                        if(c.position >= max_pos){
                            max = c.color;
                            max_pos = c.position;
                        }
                    }
                    //NVG gradient out range will use the last color
                    //So we need to set the range by position

                    grad_bds.minx += min_pos * r.w;
                    grad_bds.miny += min_pos * r.h;

                    grad_bds.maxx -= (1.0f - max_pos) * r.w;
                    grad_bds.maxy -= (1.0f - max_pos) * r.h;

                    //Let's make paint
                    GLColor in = min;
                    GLColor out = max;
                    auto paint = nvgLinearGradient(
                        nvg_ctxt,
                        grad_bds.minx,
                        grad_bds.miny,
                        grad_bds.maxx,
                        grad_bds.maxy,
                        reinterpret_cast<NVGcolor&>(in),
                        reinterpret_cast<NVGcolor&>(out)
                    );
                    nvgFillPaint(nvg_ctxt,paint);
                    nvgStrokePaint(nvg_ctxt,paint);
                }
                else if(g._type == Gradient::Radial){
                    BTK_PANIC("Unimplemented brush type");
                }
                else{
                    //Impossible
                    BTK_PANIC("Unknown brush type");
                }
                break;
            }
            case BrushType::Image:{
                //Image brush
                //Alloc a temp tex
                PixBufRef image = brush.image();
                int w = image->w;
                int h = image->h;
                int x;
                int y;
                
                auto tex = alloc_temp_tex(
                    w,h,&x,&y
                );

                //Copy to 
                uint8_t *buf_data = static_cast<uint8_t*>(image->pixels);

                uint8_t *dst_data = reinterpret_cast<uint8_t*>(tex->pixels) +
                    y * (tex->w * 4) +
                    x * 4;

                SDL_ConvertPixels(
                    w,h,
                    image->format->format,
                    buf_data,
                    image->pitch,
                    SDL_PIXELFORMAT_RGBA32,
                    dst_data,
                    tex->w * 4
                );
                //Make pattern
                Rect src = {x,y,w,h};
                auto target = sub_image_pattern(
                    tex->w,
                    tex->h,
                    src,
                    r
                );
                auto paint = nvgImagePattern(
                    nvg_ctxt,
                    target.x,
                    target.y,
                    target.w,
                    target.h,
                    0,
                    tex->tex,
                    1.0f
                );

                //Set Fill / Stroke
                nvgFillPaint(nvg_ctxt,paint);
                nvgStrokePaint(nvg_ctxt,paint);
                break;
            }
            default:
                BTK_PANIC("Unimplemented brush type");
        }
    }
}