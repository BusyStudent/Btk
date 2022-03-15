#include "build.hpp"
#include <Btk/render.hpp>
#include <Btk/svg.hpp>

#include <cstring>
#include <cstdlib>
#include <cmath>


#ifndef BTK_DISABLE_SVG

#include "libs/nanovg.h"

#define NANOSVG_IMPLEMENTATION
namespace{
    #include "libs/nanosvg.h"
}

//
// Copyright (c) 2020 Stylus Labs - see LICENSE.txt
//   based on nanovg and nanosvg:
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//   and stb_truetype:
// Public domain; authored from 2009-2020 by Sean Barrett / RAD Game Tools
//


namespace{
    //From styluslabs/nanovgXC
    static struct NVGcolor nvgColUint(unsigned int col)
    {
    struct NVGcolor c;
    c.r = (col & 0xff) / 255.0f;
    c.g = ((col >> 8) & 0xff) / 255.0f;
    c.b = ((col >> 16) & 0xff) / 255.0f;
    c.a = ((col >> 24) & 0xff) / 255.0f;
    return c;
    }

    static NVGpaint linearGradToNVGpaint(NVGcontext* vg, NSVGgradient* g)
    {
        // we only support simple (2-stop) gradients
        NSVGgradientStop* s0 = &g->stops[0];
        NSVGgradientStop* s1 = &g->stops[g->nstops - 1];
        float* t = g->xform;
        float x0 = t[4], y0 = t[5], dx = t[2], dy = t[3];
        return nvgLinearGradient(vg, x0 + s0->offset*dx, y0 + s0->offset*dy,
            x0 + s1->offset*dx, y0 + s1->offset*dy, nvgColUint(s0->color), nvgColUint(s1->color));
    }
    static NVGpaint radialGradToNVpaint(NVGcontext* vg, NSVGgradient* g){
        BTK_LOGINFO("Unsupported radial");
        NSVGgradientStop* s0 = &g->stops[0];
        NSVGgradientStop* s1 = &g->stops[1];
        float* t = g->xform;
        float x0 = t[4], y0 = t[5], dx = t[2], dy = t[3];
        return nvgRadialGradient(vg, x0 + s0->offset*dx, y0 + s0->offset*dy,
            x0 + s1->offset*dx, y0 + s1->offset*dy, nvgColUint(s0->color), nvgColUint(s1->color));
    }

    static void nsvgRender(NVGcontext* vg, const NSVGimage* image)
    {
    static int nsvgCap[] = {NVG_BUTT, NVG_ROUND, NVG_SQUARE};
    static int nsvgJoin[] = {NVG_MITER, NVG_ROUND, NVG_BEVEL};

    // float w = bounds[2] - bounds[0];
    // float h = bounds[3] - bounds[1];
    // float s = nsvg__minf(w / image->width, h / image->height);
    nvgSave(vg);
    // map SVG viewBox to window
    // nvgTransform(vg, s, 0, 0, s, bounds[0], bounds[1]);
    // now map SVG content to SVG viewBox (by applying viewBox transform)
    // nvgTransformArray(vg, image->viewXform);
    for(NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
        if(shape->fill.type == NSVG_PAINT_NONE && shape->stroke.type == NSVG_PAINT_NONE) continue;
        if(shape->paths) {
        nvgBeginPath(vg);
        for(NSVGpath* path = shape->paths; path != NULL; path = path->next) {
            nvgMoveTo(vg, path->pts[0], path->pts[1]);
            for (int i = 1; i < path->npts-1; i += 3) {
            float* p = &path->pts[i*2];
            if(fabsf((p[2] - p[0]) * (p[5] - p[1]) - (p[4] - p[0]) * (p[3] - p[1])) < 0.001f)
                nvgLineTo(vg, p[4],p[5]);
            else
                nvgBezierTo(vg, p[0],p[1], p[2],p[3], p[4],p[5]);
            }
            if(path->closed)
            nvgClosePath(vg);
            // nvgPathWinding(vg, NVG_AUTOW);
        }
        if(shape->fill.type != NSVG_PAINT_NONE) {
            if(shape->fill.type == NSVG_PAINT_COLOR){
                nvgFillColor(vg, nvgColUint(shape->fill.color));  // fill.color includes opacity
            }
            else if(shape->fill.type == NSVG_PAINT_LINEAR_GRADIENT){
                nvgFillPaint(vg, linearGradToNVGpaint(vg, shape->fill.gradient));
            }
            else{
                //NSVG_PAINT_RADIAL_GRADIENT
                nvgFillPaint(vg, radialGradToNVpaint(vg, shape->fill.gradient));
            }
            nvgFill(vg);
        }
        if (shape->stroke.type == NSVG_PAINT_COLOR) {
            nvgStrokeColor(vg, nvgColUint(shape->stroke.color));
            nvgStrokeWidth(vg, shape->strokeWidth);
            nvgLineCap(vg, nsvgCap[(int)shape->strokeLineCap]);
            nvgLineJoin(vg, nsvgJoin[(int)shape->strokeLineJoin]);
            nvgStroke(vg);
        }
        }
    }
    nvgRestore(vg);
    }

}

namespace Btk{
    //Parse String
    void *SVGImage::_Parse(const char *s,size_t n,const char *unit,float dpi){
        if(unit == nullptr){
            unit = "px";
        }
        //Create a temp buffer
        char *buffer = (char*)SDL_malloc((n + 1) * sizeof(char));
        if(buffer == nullptr){
            throwSDLError();
        }
        memcpy(buffer,s,n * sizeof(char));
        buffer[n] = '\0';

        void *image = nsvgParse(buffer,unit,dpi);
        SDL_free(buffer);

        if(image == nullptr){
            throwRuntimeError("SVGParseError");
        }
        return image;
    }

    //Parse Stream
    void *SVGImage::_ParseStream(SDL_RWops *rwops,const char *unit,float dpi){
        if(unit == nullptr){
            unit = "px";
        }
        //Get file size
        auto cur = SDL_RWtell(rwops);
        SDL_RWseek(rwops,0,RW_SEEK_END);
        Sint64 n = (SDL_RWtell(rwops) - cur);
        //Reset back
        SDL_RWseek(rwops,cur,RW_SEEK_SET);
        //Create a temp buffer
        char *buffer = (char*)SDL_malloc((n + 1) * sizeof(char));
        if(buffer == nullptr){
            throwSDLError();
        }
        if(SDL_RWread(rwops,buffer,n,1) != 1){
            SDL_free(buffer);
            throwSDLError();
        }
        buffer[n] = '\0';

        void *image = nsvgParse(buffer,unit,dpi);
        SDL_free(buffer);

        if(image == nullptr){
            throwRuntimeError("SVGParseError");
        }
        return image;
    }
    void *SVGImage::_ParseFile(u8string_view filename,const char *unit,float dpi){
        auto rw = RWops::FromFile(u8string(filename).c_str(),"r");
        return _ParseStream(rw.get(),unit,dpi);
    }
    void SVGImage::_Free(void *i){
        nsvgDelete(static_cast<NSVGimage*>(i));
    }
    void SVGImage::_Render(const void *_image,Renderer &renderer){
        auto image = static_cast<const NSVGimage*>(_image);
        auto ctxt  = renderer.context();
        nsvgRender(ctxt,image);
    }
    FSize SVGImage::_Query(const void *_image){
        if(_image == nullptr){
            return {-1,-1};
        }
        const NSVGimage *image = static_cast<const NSVGimage*>(_image);
        return {image->width,image->height};
    }
    //SVGView
    SVGView::SVGView(){}
    SVGView::~SVGView(){}
    void SVGView::draw(Renderer &renderer,Uint32){
        if(not image.empty()){
            //Save the status
            renderer.save();
            //Limited to bound
            if(limited_to_bound){
                renderer.intersest_scissor(rect);
            }
            //Begin translate and scale
            renderer.translate(x(),y());
            renderer.scale(scale_x,scale_y);

            image.render(renderer);
            renderer.restore();
        }
    }
}

#endif