#include "../build.hpp"

#include <Btk/exception.hpp>
#include <Btk/render.hpp>
#include <Btk/font.hpp>
#include <memory>

extern "C"{
    #define FONS_USE_FREETYPE
    #define NVG_NO_STB
    #include "../libs/nanovg.h"
    #include "../libs/nanovg.c"
    #include "../libs/fontstash.h"
}



//Our nvg method

extern "C"{
    /**
     * @brief This function like nvgBeginFrame but is didnot clear the state
     * 
     * @param ctx 
     * @param windowWidth 
     * @param windowHeight 
     * @param devicePixelRatio 
     */
    void nvgBeginFrameEx(NVGcontext* ctx, float windowWidth, float windowHeight, float devicePixelRatio){
    /*	printf("Tris: draws:%d  fill:%d  stroke:%d  text:%d  TOT:%d\n",
		ctx->drawCallCount, ctx->fillTriCount, ctx->strokeTriCount, ctx->textTriCount,
		ctx->fillTriCount+ctx->strokeTriCount+ctx->textTriCount);*/

	// ctx->nstates = 0;
	// nvgSave(ctx);
	// nvgReset(ctx);

	nvg__setDevicePixelRatio(ctx, devicePixelRatio);

	ctx->params.renderViewport(ctx->params.userPtr, windowWidth, windowHeight, devicePixelRatio);

	ctx->drawCallCount = 0;
	ctx->fillTriCount = 0;
	ctx->strokeTriCount = 0;
	ctx->textTriCount = 0;
    }
}

#define NVG_CHECK(OUR,NVG) \
    static_assert(int(Btk::OUR) == NVG,"Broken defines in render.hpp")
#define NVG_CHECK_SIZE(OUR,NVG) \
    static_assert(sizeof(Btk::OUR) == sizeof(NVG),"Broken defines in render.hpp")
#define NVG_CHECK_OFSET(OUR,NVG,ELEM) \
    static_assert(offsetof(Btk::OUR,ELEM) == offsetof(NVG,ELEM),"Broken defines in render.hpp")

//Check const

NVG_CHECK(LineCap::Butt,NVG_BUTT);
NVG_CHECK(LineCap::Round,NVG_ROUND);
NVG_CHECK(LineCap::Square,NVG_SQUARE);

NVG_CHECK(LineJoin::Round,NVG_ROUND);
NVG_CHECK(LineJoin::Bevel,NVG_BEVEL);
NVG_CHECK(LineJoin::Miter,NVG_MITER);

NVG_CHECK(TextAlign::Left,NVG_ALIGN_LEFT);
NVG_CHECK(TextAlign::Center,NVG_ALIGN_CENTER);
NVG_CHECK(TextAlign::Right,NVG_ALIGN_RIGHT);
NVG_CHECK(TextAlign::Top,NVG_ALIGN_TOP);
NVG_CHECK(TextAlign::Middle,NVG_ALIGN_MIDDLE);
NVG_CHECK(TextAlign::Bottom,NVG_ALIGN_BOTTOM);
NVG_CHECK(TextAlign::Baseline,NVG_ALIGN_BASELINE);

NVG_CHECK(PathWinding::CW,NVG_CW);
NVG_CHECK(PathWinding::CCW,NVG_CCW);
//Check size
NVG_CHECK_SIZE(RendererPaint,NVGpaint);
NVG_CHECK_SIZE(GLColor,NVGcolor);

//Check color's offset
NVG_CHECK_OFSET(GLColor,NVGcolor,r);
NVG_CHECK_OFSET(GLColor,NVGcolor,g);
NVG_CHECK_OFSET(GLColor,NVGcolor,b);
NVG_CHECK_OFSET(GLColor,NVGcolor,a);

namespace Btk{
    //USE The nvg
    static size_t BtkTextGlyphPositions(
        NVGcontext* ctx,
        float x,float y,
        u8string_view str,
        bool (*callback)(const GlyphPosition &,void *user),
        void *user
    ){
	NVGstate* state = nvg__getState(ctx);
	float scale = nvg__getFontScale(state) * ctx->devicePxRatio;
	float invscale = 1.0f / scale;
	FONStextIter iter, prevIter;
	FONSquad q;
    GlyphPosition pos;//Our position
	size_t npos = 0;

	if (state->fontId == FONS_INVALID) return 0;

    const char *string = &*str.base().begin();
    const char *end = &*str.base().end();

	if (string == end)
		return 0;

	fonsSetSize(ctx->fs, state->fontSize*scale);
	fonsSetSpacing(ctx->fs, state->letterSpacing*scale);
	fonsSetBlur(ctx->fs, state->fontBlur*scale);
	fonsSetAlign(ctx->fs, state->textAlign);
	fonsSetFont(ctx->fs, state->fontId);

	fonsTextIterInit(ctx->fs, &iter, x*scale, y*scale, string, end, FONS_GLYPH_BITMAP_OPTIONAL);
	prevIter = iter;
	while (fonsTextIterNext(ctx->fs, &iter, &q)) {
		if (iter.prevGlyphIndex < 0 && nvg__allocTextAtlas(ctx)) { // can not retrieve glyph?
			iter = prevIter;
			fonsTextIterNext(ctx->fs, &iter, &q); // try again
		}
		prevIter = iter;
		pos.str = iter.str;
		pos.x = iter.x * invscale;
		pos.minx = nvg__minf(iter.x, q.x0) * invscale;
		pos.maxx = nvg__maxf(iter.nextx, q.x1) * invscale;
        pos.glyph = iter.codepoint;
		npos++;
        //Call the callback
		if (callback(pos,user) == false)
			break;
	}
	return npos;
    }
}


















namespace Btk{
    RendererDevice::~RendererDevice() = default;
    Font Renderer::cur_font(){
        auto state = nvg__getState(nvg_ctxt);
        int idx = state->fontId;
        if(idx == FONS_INVALID){
            throwRuntimeError("Invaid font");
        }
        auto i = BtkFt_GetFromID(idx);
        if(i == nullptr){
            throwRuntimeError("Invaid font");
        }
        return Font(BtkFt_Dup(i),state->fontSize);
    }
    void Renderer::flush(){
        nvg_ctxt->params.renderFlush(nvg_ctxt->params.userPtr);
    }
    //Our text method
    size_t Renderer::glyph_position(
        float x,float y,
        u8string_view text,
        bool (*callback)(const GlyphPosition &,void *user),
        void *user
        ){
        
        return BtkTextGlyphPositions(
            nvg_ctxt,
            x,y,
            text,
            callback,
            user
        );
    }
     //Set font
    bool Renderer::use_font(u8string_view fontname) noexcept{
        int font_id = BtkFt_GetID(BtkFt_GlobalFind(fontname));
        if(font_id == -1){
            return false;
        }
        nvgFontFaceId(nvg_ctxt,font_id);
        return true;
    }
    bool Renderer::use_font(const Font &font){
        nvgFontFaceId(nvg_ctxt,0);
        nvgFontSize(nvg_ctxt,font.ptsize());
        return true;
    }
    //Default device operations
    void RendererDevice::begin_frame(Context ctxt,float w,float h,float ratio){
        nvgBeginFrame(ctxt,w,h,ratio);
    }
    void RendererDevice::begin_frame_ex(Context ctxt,float w,float h,float ratio){
        nvgBeginFrameEx(ctxt,w,h,ratio);
    }
    void RendererDevice::end_frame(Context ctxt){
        nvgEndFrame(ctxt);
    }
    void RendererDevice::cancel_frame(Context ctxt){
        nvgCancelFrame(ctxt);
    }
    TextureID RendererDevice::create_texture(Context ctxt,int w,int h,TextureFlags flags,const void *p){
        return nvgCreateImageRGBA(ctxt,w,h,int(flags),static_cast<const unsigned char*>(p));
    }
    TextureID RendererDevice::create_texture_from(Context ctxt,SDL_Surface *surf,TextureFlags flags){
        if(surf == nullptr){
            return -1;
        }
        if(surf->format->format != PixelFormat::RGBA32){
            //Convert format
            SDL_Surface *new_surf = SDL_ConvertSurfaceFormat(
                surf,
                PixelFormat::RGBA32,
                0
            );
            if(new_surf == nullptr){
                throwSDLError();
            }
            //Make guard
            std::unique_ptr<SDL_Surface,decltype(SDL_FreeSurface)*> guard(
                new_surf,
                SDL_FreeSurface
            );
            return create_texture_from(ctxt,new_surf,flags);
        }
        if(SDL_MUSTLOCK(surf)){
            SDL_LockSurface(surf);
        }
        //Make guard
        auto unlock_fn = [](SDL_Surface *surf){
            if(SDL_MUSTLOCK(surf)){
                SDL_UnlockSurface(surf);
            }
        };
        std::unique_ptr<SDL_Surface,decltype(unlock_fn)> guard(surf,unlock_fn);
        return create_texture(ctxt,surf->w,surf->h,flags,surf->pixels);
    }
    void RendererDevice::update_texture(Context ctxt,TextureID id,const Rect *r,const void *pixels){
        Rect rect = {0,0,0,0};

        if(r == nullptr){
            nvgImageSize(ctxt,id,&rect.w,&rect.h);
        }
        else{
            //Copy it
            rect = *r;
        }
        //Call 
        ctxt->params.renderUpdateTexture(
            ctxt->params.userPtr,
            id,
            rect.x,
            rect.y,
            rect.w,
            rect.h,
            static_cast<const unsigned char*>(pixels)
        );
    }

    void *RendererDevice::lock_texture(
        Context ctxt,
        TextureID id,
        const Rect *r,
        LockFlag flag   
    ){
        //Default only support write
        if(flag != Write){
            return nullptr;
        }
        Rect rect;
        if(r == nullptr){
            rect.x = 0;
            rect.y = 0;
            auto [w,h] = texture_size(ctxt,id);
            rect.w = w;
            rect.h = h;
        }
        else{
            rect = *r;
        }
        //Alloc mem
        void *mem = SDL_malloc(
            sizeof(Rect) + SDL_BYTESPERPIXEL(PixelFormat::RGBA32) * rect.w * rect.h
        );
        if(mem == nullptr){
            return nullptr;
        }
        *static_cast<Rect*>(mem) = rect;
        return static_cast<Uint8*>(mem) + sizeof(Rect);
    }
    void  RendererDevice::unlock_texture(Context ctxt,TextureID id,void *pixels){
        if(pixels != nullptr){
            Rect *rect = reinterpret_cast<Rect*>(
                static_cast<Uint8*>(pixels) - sizeof(Rect)
            );
            update_texture(ctxt,id,rect,pixels);
            SDL_free(rect);
        }
    }
    bool  RendererDevice::configure_texture(Context,TextureID,const TextureFlags *){
        return false;
    }
    void  RendererDevice::destroy_texture(Context ctxt,TextureID id){
        nvgDeleteImage(ctxt,id);
    }
}