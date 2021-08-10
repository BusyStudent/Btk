#include "../build.hpp"

#include <Btk/font/font.hpp>
#include <Btk/render.hpp>
#include <Btk/font.hpp>

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
    Font Renderer::cur_font(){
        int idx = nvg__getState(nvg_ctxt)->fontId;
        if(idx == FONS_INVALID){
            throwRuntimeError("Invaid font");
        }
        auto i = fontsGetFaceByID(nvg_ctxt->fs,idx);
        if(i == nullptr){
            throwRuntimeError("Invaid font");
        }
        auto p = static_cast<Ft::Font*>(i);
        p->ref();
        return p;
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
    void RendererDevice::destroy_texture(Context ctxt,TextureID id){
        nvgDeleteImage(ctxt,id);
    }
}