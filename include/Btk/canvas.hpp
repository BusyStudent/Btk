#if !defined(_BTK_CANVAS_HPP_)
#define _BTK_CANVAS_HPP_
#include "defs.hpp"
#include "widget.hpp"
#include "container.hpp"

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
    //Shapes --begin
    class BTKAPI ShapeNode:public Widget{
        public:
            ShapeNode();
            ShapeNode(const ShapeNode &) = delete;
            ~ShapeNode();

            virtual FPolygen polygen() const = 0;


        private:
            Signal<void()> signal_clicked;
            Signal<void()> signal_collided;
    };
    class BTKAPI ShapeRectNode:public ShapeNode{
        private:
            Color c;
    };
    class BTKAPI ShapeLineNode:public ShapeNode{
        
    };
    class BTKAPI ShapePolygenNode:public ShapeNode{
        public:
            ShapePolygenNode();
            ~ShapePolygenNode();

            void draw(Renderer&) override;

            bool handle_mouse(MouseEvent &) override;
            bool handle_motion(MotionEvent &) override;

        private:
            FPolygen poly = {};
            Color c = {};
            bool fill = false;
    };
    class BTKAPI ShapeImageNode:public ShapeNode{

    };
    class BTKAPI ShapeTextNode:public ShapeNode{

    };
    //Shapes --end
    class BTKAPI SceneView:public Group{
        public:
            SceneView();
            ~SceneView();
        private:
            bool _relative_coordinate = true;
    };
    class BTKAPI Graph:public Widget{
        private:
            Color _background_color;
            Color _text_color;
    };
    class GLDevice;
    /**
     * @brief Basic class for using OpenGL
     * 
     */
    class BTKAPI GLCanvas:public Widget{
        public:
            // virtual ~GLCanvas();
            virtual void set_parent(Widget *) override;
            virtual void draw(Renderer&) final;
            /**
             * @brief Virtual method for OpenGL Drawing
             * 
             */
            virtual void gl_draw() = 0;
            virtual void gl_init() = 0;
            virtual void gl_cleanup() = 0;
            /**
             * @brief Get the OpenGL Device 
             * 
             * @return GLDevice* 
             */
            GLDevice *gl_device();
        private:
            
            bool _is_inited = false;
    };
    #if BTK_STILL_DEV
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
    #endif
}

#endif // _BTK_CANVAS_HPP_
