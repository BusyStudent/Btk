#include "build.hpp"
#include <Btk/render.hpp>
#include <Btk/svg.hpp>

#include <cstring>
#include <cstdlib>
#include <cmath>


#ifndef BTK_DISABLE_SVG

#include "libs/nanovg.h"

namespace{
    #define NANOSVG_IMPLEMENTATION
    #include "libs/nanosvg.h"
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
    void *SVGImage::_ParseFile(const char *f,const char *unit,float dpi){
        if(unit == nullptr){
            unit = "px";
        }
        void *image = nsvgParseFromFile(f,unit,dpi);
        if(image == nullptr){
            throwRuntimeError("SVGParseError");
        }
        return image;
    }
    //Parse Stream
    // void *SVGImage::_ParseStream(SDL_RWops *rwops,const char *unit,float dpi){
    //     if(unit == nullptr){
    //         unit = "px";
    //     }
    //     //Create a temp buffer
    //     char *buffer = (char*)SDL_malloc((n + 1) * sizeof(char));
    //     if(buffer == nullptr){
    //         throwSDLError();
    //     }
    //     memcpy(buffer,s,n * sizeof(char));
    //     buffer[n] = '\0';

    //     void *image = nsvgParse(buffer,unit,dpi);
    //     SDL_free(buffer);

    //     if(image == nullptr){
    //         throwRuntimeError("SVGParseError");
    //     }
    //     return image;
    // }
    void SVGImage::_Free(void *i){
        nsvgDelete(static_cast<NSVGimage*>(i));
    }
    void SVGImage::_Render(const void *_image,Renderer &renderer){
        if(_image == nullptr){
            return;
        }
        const NSVGimage *image = static_cast<const NSVGimage*>(_image);
        //Helper
        //Code from ZgeNano
        auto create_linear_paint = [&](NSVGgradient *gradient){
            float inverse[6];
            float sx, sy, ex, ey;

            nvgTransformInverse(inverse, gradient->xform);
            nvgTransformPoint(&sx, &sy, inverse, 0, 0);
            nvgTransformPoint(&ex, &ey, inverse, 0, 1);

            auto in = GetRGBA32f(gradient->stops[0].color);
            auto out = GetRGBA32f(gradient->stops[gradient->nstops - 1].color);

            return renderer.linear_gradient( sx, sy, ex, ey,
                in,
                out
            );
        };
        auto create_radial_paint = [&](NSVGgradient *gradient){
            float inverse[6];
            float cx, cy, r1, r2, inr, outr;

            nvgTransformInverse(inverse, gradient->xform);
            nvgTransformPoint(&cx, &cy, inverse, 0, 0);
            nvgTransformPoint(&r1, &r2, inverse, 0, 1);
            outr = r2 - cy;
            if (gradient->nstops == 3)
                inr = gradient->stops[1].offset * outr;
            else
                inr = 0;

            auto in = GetRGBA32f(gradient->stops[0].color);
            auto out = GetRGBA32f(gradient->stops[gradient->nstops - 1].color);

            return renderer.radial_gradient(cx, cy, inr, outr,
                in,
                out
            );
        };
        auto convert_linecap = [](int linecap){
            switch(linecap){
                case NSVG_CAP_BUTT:
                    return LineCap::Butt;
                case NSVG_CAP_ROUND:
                    return LineCap::Round;
                case NSVG_CAP_SQUARE:
                    return LineCap::Square;
                default:
                    return LineCap::Butt;
            }
        };
        auto convert_linejoin = [](int linejoin){
            switch(linejoin){
                case NSVG_JOIN_BEVEL:
                    return LineJoin::Bevel;
                case NSVG_JOIN_MITER:
                    return LineJoin::Miter;
                case NSVG_JOIN_ROUND:
                    return LineJoin::Round;
                default:
                    return LineJoin::Miter;
            }
        };
        //Begin
        renderer.save();
        for(auto shape = image->shapes;shape != nullptr;shape = shape->next){
            //Skip if
            if((shape->flags & NSVG_FLAGS_VISIBLE) != NSVG_FLAGS_VISIBLE){
                continue;
            }
            renderer.set_alpha(shape->opacity);
            //Begin configure stroke and fill
            if(shape->stroke.type){
                //Stroke
                switch(shape->stroke.type){
                    case NSVG_PAINT_COLOR:
                        renderer.stroke_color(GetRGBA32(shape->stroke.color));
                        break;
                    case NSVG_PAINT_LINEAR_GRADIENT:
                        renderer.stroke_paint(create_linear_paint(shape->stroke.gradient));
                        break;
                    case NSVG_PAINT_RADIAL_GRADIENT:
                        renderer.stroke_paint(create_radial_paint(shape->stroke.gradient));
                        break;
                }
                renderer.set_linecap(convert_linecap(shape->strokeLineCap));
                renderer.set_linejoin(convert_linejoin(shape->strokeLineJoin));
                renderer.set_miterlimit(shape->miterLimit);
                renderer.stroke_width(shape->strokeWidth);
            }
            else if(shape->fill.type){
                //Fill
                switch(shape->fill.type){
                    case NSVG_PAINT_COLOR:
                        renderer.fill_color(GetRGBA32(shape->fill.color));
                        break;
                    case NSVG_PAINT_LINEAR_GRADIENT:
                        renderer.fill_paint(create_linear_paint(shape->fill.gradient));
                        break;
                    case NSVG_PAINT_RADIAL_GRADIENT:
                        renderer.fill_paint(create_radial_paint(shape->fill.gradient));
                        break;
                }
            }
            //TODO It still has some error in fill
            //For all path and draw it
            for(auto path = shape->paths;path != nullptr;path = path->next){
                //Draw path
                renderer.begin_path();
                renderer.move_to(path->pts[0],path->pts[1]);
                for (int i = 0;i < path->npts-1;i += 3) {
                    float* p = &path->pts[i*2];
                    renderer.bezier_to(p[2], p[3], p[4], p[5], p[6], p[7]);
                }

                if(path->closed){
                    renderer.close_path();
                }
                //Do it
                if(shape->stroke.type){
                    renderer.stroke();
                }
                else if(shape->fill.type){
                    renderer.fill();
                }
            }

        }
        renderer.restore();
        //End
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
    void SVGView::draw(Renderer &renderer){
        if(not image.empty()){
            //Save the status
            renderer.save();
            //Limited to bound
            if(limited_to_bound){
                renderer.intersest_scissor(rect);
            }
            //Begin translate and scale
            renderer.translate(x(),y());

            image.render(renderer);
            renderer.restore();
        }
    }
}

#endif