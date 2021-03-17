#if !defined(_BTK_CANVAS_HPP_)
#define _BTK_CANVAS_HPP_
#include "defs.hpp"
#include "render.hpp"
#include "widget.hpp"

namespace Btk{
    class BTKAPI Canvas:public Widget{
        public:
            //Renderer function
            typedef Function<void(Renderer&)> DrawFn;
            typedef Function<bool(Event&)>    EventFn;
        public:
            Canvas();
            Canvas(int x,int y,int w,int h);
            Canvas(const Canvas &) = delete;
            ~Canvas();

            void draw(Renderer&);
            bool handle(Event &);

            EventFn &handle(){
                return event_fn;
            }
            DrawFn &draw(){
                return draw_fn;
            }
            using Widget::redraw;
        private:
            EventFn event_fn;
            DrawFn  draw_fn;
    };
}

#endif // _BTK_CANVAS_HPP_
