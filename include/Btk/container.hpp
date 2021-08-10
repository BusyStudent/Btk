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
            //Platform 
            using _XDisplay = void *;
            using _XWindow  = unsigned long;
            using _HWND = void *;

            #ifdef _WIN32
            using WinPtr = _HWND;
            #else
            struct WinPtr:public std::pair<_XDisplay,_XWindow>{
                using std::pair<_XDisplay,_XWindow>::pair;
            };
            #endif
            EmbedWindow();
            EmbedWindow(const EmbedWindow &) = delete;
            ~EmbedWindow();
            
            void draw(Renderer &) override;
            void set_rect(const Rect &r) override;
            void set_parent(Widget *w) override;
            /**
             * @brief Set the window object
             * 
             * @param window 
             */
            void set_window(WinPtr window);
            #ifdef __gnu_linux__
            /**
             * @brief X11 Set the window object
             * 
             * @param win The XWindowID
             */
            void set_window(_XWindow win){
                //Use the helper window's display
                WinPtr p = _helper_window;
                p.second = win;
            }
            #endif
            /**
             * @brief Has window embed 
             * 
             * @return true 
             * @return false 
             */
            bool has_embed() const noexcept{
                return _user_window != WinPtr{};
            }
            /**
             * @brief Detach the window embed in
             * 
             */
            void detach_window();
            /**
             * @brief Get the window which is embed in the widget
             * 
             * @return WinPtr 
             */
            WinPtr embed_window() const{
                return _user_window;
            }
            WinPtr helper_window();

        private:
            /**
             * @brief Attach to the parent window
             * @param rect The children's rect relality to the parent 
             */
            BTKHIDDEN
            void nt_attach(const Rect &rect);
            BTKHIDDEN
            void nt_set_rect(const Rect &r);
            /**
             * @brief The window witch want to be embed
             * 
             */
            WinPtr _user_window = {};
            WinPtr _target_window = {};
            WinPtr _helper_window = {};
            /**
             * @brief Helper window for clip the screen
             * 
             */
            SDL_Window *_window = nullptr;
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
}

#endif // _BTK_CONTAINER_HPP_
