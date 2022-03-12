#if !defined(_BTK_CONTAINER_HPP_)
#define _BTK_CONTAINER_HPP_
/**
 * @brief This header include many useful containers
 * 
 */
struct SDL_Window;
#include "defs.hpp"
#include "widget.hpp"
namespace Btk{
    class NativeWindow;
    enum class WindowFlags:Uint32;
    /**
     * @brief A Interface for manage widget
     * 
     */
    class BTKAPI Group:public Container{
        public:
            Group() = default;
            ~Group() = default;
        public:
            void draw(Renderer             &) override;
            bool handle(Event              &) override;
            bool handle_drop(DropEvent     &) override;
            bool handle_drag(DragEvent     &) override;
            bool handle_mouse(MouseEvent   &) override;
            bool handle_wheel(WheelEvent   &) override;
            bool handle_motion(MotionEvent &) override;
            bool handle_keyboard(KeyEvent  &) override;
            bool handle_textinput(TextInputEvent     &) override;
            bool handle_textediting(TextEditingEvent &) override;

            bool detach(Widget *w) override;
            /**
             * @brief Set the focus widget object
             * 
             */
            bool set_focus_widget(Widget *);
        private:
            //< The current dragging widget
            Widget *drag_widget = nullptr;
            //< The widget which has focus
            Widget *focus_widget = nullptr;
            //< Current Widget mouse or finger point at
            Widget *cur_widget = nullptr;
            bool drag_rejected = false;
    };
    class BTKAPI GroupBox:public Group{
        public:
            GroupBox();
            ~GroupBox();
            
            void draw(Renderer &) override;
            // bool handle(Event  &) override;
        private:
            Color borader_color;
            Color background_color;

            bool draw_boarder = true;
            bool draw_background = true;
    };
    class BTKAPI DockWidget:public Group{
        
    };
    class BTKAPI ToolWidget:public Group{
        
    };
    class BTKAPI ScrollArea:public Group{
        
    };
    class BTKAPI TabWidget:public Group{
        
    };
    class BTKAPI StackedWidget:public Group{

    };
    /**
     * @brief A virtual window
     * 
     */
    class BTKAPI VirtualWindow:public Group{

    };
    #if BTK_STILL_DEV
    /**
     * @brief Embed Native Window on here
     * 
     */
    class BTKAPI EmbedWindow:public Widget{
        public:
            EmbedWindow();
            ~EmbedWindow();

            void draw(Renderer &) override;
            void set_rect(const Rect &) override;
            void set_parent(Widget *) override;
            void set_window(NativeWindow *win);
        private:
            struct _Internal;
            _Internal *internal;
    };
    /**
     * @brief This Widget like GLCanvas but has independ gl context
     * 
     */
    class BTKAPI GLWidget:public EmbedWindow{
        public:
            /**
             * @brief Init OpenGL Context
             * 
             */
            void init();
            void draw(Renderer &) override final;
            /**
             * @brief Draw by OpenGL Context
             * 
             */
            virtual void gl_draw() = 0;
        private:
            void *sdl_window;
            void *gl_context;
    };
    #endif
}

#endif // _BTK_CONTAINER_HPP_
