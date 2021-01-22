#if !defined(_BTK_CANVAS_HPP_)
#define _BTK_CANVAS_HPP_
#include "defs.hpp"
#include "render.hpp"
#include "widget.hpp"
#include "signal/function.hpp"

namespace Btk{
    class BTKAPI Canvas:public Widget{
        public:
            //Renderer function
            typedef Function<void(Renderer&)> DrawFn;
            typedef Function<bool(Event&)>    EventFn;
        public:
            Canvas(Container&);
            Canvas(Container&,int x,int y,int w,int h);
            Canvas(const Canvas &) = delete;
            ~Canvas();

            void draw(Renderer&);
            bool handle(Event &);
        private:
            EventFn event_fn;
            DrawFn  draw_fn;
    };
}

#endif // _BTK_CANVAS_HPP_