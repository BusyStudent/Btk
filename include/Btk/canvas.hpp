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
            //Save / Restore before / after draw
            bool protect_context = true;
    };
    class GLDevice;
    /**
     * @brief Basic class for using OpenGL
     * 
     */
    class BTKAPI GLCanvas:public Widget{
        public:
            virtual void draw(Renderer&) final;
            /**
             * @brief Virtual method for OpenGL Drawing
             * 
             */
            virtual void gl_draw() = 0;
            /**
             * @brief Get the OpenGL Device 
             * 
             * @return GLDevice* 
             */
            GLDevice *gl_device();
    };
    /**
     * @brief Basic class for using Direct3D 11
     * 
     */
    class BTKAPI DxCanvas{
        public:
            virtual void draw(Renderer&) final;
            virtual void dx_draw() = 0;

    };
    class BTKAPI VkCanvas{
        public:
            virtual void draw(Renderer&) final;
            virtual void vk_draw() = 0;
    };
}

#endif // _BTK_CANVAS_HPP_
