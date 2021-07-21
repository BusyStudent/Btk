#if !defined(_BTK_CONTAINER_HPP_)
#define _BTK_CONTAINER_HPP_
/**
 * @brief This header include many useful containers
 * 
 */
#include "defs.hpp"
#include "widget.hpp"
namespace Btk{
    class NativeWindow;
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
            bool handle_drag(DragEvent     &) override;
            bool handle_mouse(MouseEvent   &) override;
            bool handle_wheel(WheelEvent   &) override;
            bool handle_motion(MotionEvent &) override;
            bool handle_keyboard(KeyEvent  &) override;
            bool handle_textinput(TextInputEvent &) override;

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
     * @brief Embed Native Window on here
     * 
     */
    class BTKAPI EmbedWindow:public Widget{
        public:
            EmbedWindow() = default;
            
            void set_rect(const Rect &r) override;

            #ifdef _WIN32

            void set_window(NativeWindow *win);

            #elif defined(__gnu_linux__)
            void set_window(void *display,unsigned long window);
            #endif
        private:
            #ifdef _WIN32
            NativeWindow *winptr = nullptr;

            #else defined(__gnu_linux__)

            void *_display;
            void *_window;

            #endif
    };
}

#endif // _BTK_CONTAINER_HPP_
