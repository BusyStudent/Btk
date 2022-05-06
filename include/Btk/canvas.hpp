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

            void draw(Renderer&,Uint32) override;
            bool handle(Event &) override;

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

    #ifndef BTK_DISABLE_SHAPE_API
    //Shapes --begin
    class BTKAPI ShapeNode:public Widget{
        public:
            ShapeNode();
            ShapeNode(const ShapeNode &) = delete;
            ~ShapeNode();

            /**
             * @brief Get Polygen of the shape(default empty)
             * 
             * @return FPolygen 
             */
            virtual FPolygen polygen() const;
            /**
             * @brief Get the bounding box of the shape
             * 
             * @return FRect 
             */
            virtual FRect bounding_box() const;

            bool handle(Event &) override;
            bool handle_drag(DragEvent  &) override;
            /**
             * @brief Handle mouse event
             * 
             * @return true 
             * @return false 
             */
            bool handle_mouse(MouseEvent &) override;

            //Expose signals
            Signal<void()> &signal_clicked() noexcept{
                return _signal_clicked;
            }
            Signal<void()> &signal_collided() noexcept{
                return _signal_collided;
            }
            /**
             * @brief Signal of mouse enter the shape area
             * 
             * @return Signal<void()>& 
             */
            Signal<void()> &signal_enter() noexcept{
                return _signal_enter;
            }
            /**
             * @brief Signal of mouse leave the shape area
             * 
             * @return Signal<void()>& 
             */
            Signal<void()> &signal_leave() noexcept{
                return _signal_leave;
            }
            /**
             * @brief Allow to move the shape by dragging
             * 
             * @param dragable 
             */
            void set_dragable(bool dragable = true){
                _dragable = dragable;
            }
        private:
            Signal<void()> _signal_clicked;
            Signal<void()> _signal_collided;
            Signal<void()> _signal_enter;
            Signal<void()> _signal_leave;

            bool _collided = false;
            bool _mouse_in = false;
            bool _dragable = false;
        protected:
            void notify_mouse_enter(){
                if(not _mouse_in){
                    _mouse_in = true;
                    _signal_enter.defer_emit();
                }
            }
            void notify_mouse_leave(){
                if(_mouse_in){
                    _mouse_in = false;
                    _signal_leave.defer_emit();
                }
            }
    };
    class BTKAPI ShapeRectNode:public ShapeNode{
        public:
            ShapeRectNode();
            ShapeRectNode(Color co):c(co){}
            ShapeRectNode(const ShapeRectNode &) = delete;
            ~ShapeRectNode();

            bool handle(Event &) override;

            void draw(Renderer&,Uint32) override;
            void set_color(const Color &color){
                c = color;
                redraw();
            }
            void set_fill(bool v = true){
                fill = v;
                redraw();
            }
        private:
            bool fill = false;
            Color c = {0,0,0};
    };
    class BTKAPI ShapeLineNode:public ShapeNode{
        public:
            ShapeLineNode();
            ShapeLineNode(FPoint beg,FPoint end){
                line = FLine(beg,end);
                set_rect(bounding_box());
            }
            ShapeLineNode(float x1,float y1,float x2,float y2){
                line = FLine(x1,y1,x2,y2);
                set_rect(bounding_box());
            }
            ShapeLineNode(const ShapeLineNode &) = delete;
            ~ShapeLineNode();

            void draw(Renderer&,Uint32) override;
            
            bool handle_mouse(MouseEvent &) override;
            bool handle_motion(MotionEvent &) override;

            FRect bounding_box() const override;
            /**
             * @brief Set the line object
             * 
             * @param l 
             */
            void set_line(const FLine &l){
                line = l;
                set_rect(bounding_box());
                redraw();
            }
        private:
            FLine line = {};
            Color c = {0,0,0};
            float width = 1;

            bool in_line = false;
    };
    class BTKAPI ShapePolygenNode:public ShapeNode{
        public:
            ShapePolygenNode();
            ~ShapePolygenNode();

            void draw(Renderer&,Uint32) override;

            bool handle_mouse(MouseEvent &) override;
            bool handle_motion(MotionEvent &) override;

        private:
            FPolygen poly = {};
            Color c = {};
            bool fill = false;
    };
    class BTKAPI ShapeImageNode:public ShapeNode{
        public:
            ShapeImageNode();
            ~ShapeImageNode();

            void draw(Renderer&,Uint32) override;
            /**
             * @brief Set the image object(it will take a reference)
             * 
             * @param image 
             */
            void set_image(PixBufRef image);
            /**
             * @brief Get the image
             * 
             * @return PixBufRef 
             */
            auto image() const -> PixBufRef{
                return buf;
            }
        private:
            Texture texture;
            PixBuf buf;
            bool dirty = false;
    };
    class BTKAPI ShapeTextNode:public ShapeNode{
        public:
            void draw(Renderer&,Uint32) override;
        private:
            u8string text;
            Color c;
    };
    //Shapes --end
    #endif

    class BTKAPI SceneView:public Group{
        public:
            SceneView();
            ~SceneView();
        private:
            bool _relative_coordinate = true;
            bool _clipping = true;
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
    #ifdef BTK_HAVE_DIRECTX_DEVICE
    class DxDevice;
    /**
     * @brief Basic class for using Direct3D 11
     * @note Remember to cleanup resources in destructor
     */
    class BTKAPI DxCanvas:public Widget{
        public:
            virtual void draw(Renderer&,Uint32 t) final;
            virtual void dx_draw(Uint32 time) = 0;
            /**
             * @brief Ask you,the device is ready,init the resource
             * 
             */
            virtual void dx_init() = 0;
            /**
             * @brief Ask you,the device is ready,cleanup the resource
             * 
             */
            virtual void dx_release() = 0;
            virtual void set_parent(Widget *) final;

            DxDevice *dx_device() const;
        private:
            void prepare_device();

            bool is_inited = false;
    };
    #endif

    #if BTK_STILL_DEV
    class BTKAPI VkCanvas{
        public:
            virtual void draw(Renderer&) final;
            virtual void vk_draw() = 0;
    };
    #endif
}

#endif // _BTK_CANVAS_HPP_
