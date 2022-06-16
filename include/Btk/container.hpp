#if !defined(_BTK_CONTAINER_HPP_)
#define _BTK_CONTAINER_HPP_
/**
 * @brief This header include many useful containers
 * 
 */
struct SDL_Window;
#include "defs.hpp"
#include "widget.hpp"

#define BTK_TABWIDGET_TAB_NAME "_tabname_"
#define BTK_STACKEDWIDGET_INDEX "_sidx_"

//Shadow utils
//Hide all container's method
#define BTK_SHADOW_CONTAINER_METHOD() \
    BTK_SHADOW_METHOD(add);\
    BTK_SHADOW_METHOD(remove);\
    BTK_SHADOW_METHOD(detach);\
    BTK_SHADOW_METHOD(clear);\
    BTK_SHADOW_METHOD(for_each);\
    BTK_SHADOW_METHOD(raise_widget);\
    BTK_SHADOW_METHOD(lower_widget);\
    BTK_SHADOW_METHOD(move_widget);\
    BTK_SHADOW_METHOD(index_widget);\
    BTK_SHADOW_METHOD(find_children);\

namespace Btk{
    class NativeWindow;
    enum class WindowFlags:Uint32;
    /**
     * @brief Widget for deletegate
     * 
     */
    class BTKAPI DelegateWidget:public Widget{
        public:
            DelegateWidget() = default;
            DelegateWidget(const DelegateWidget &) = delete;
            ~DelegateWidget() = default;
            //Delegate all method to 
            void set_parent(Widget  *w);
            void set_rect(const Rect &);

            void draw(Renderer &,Uint32     ) override;

            bool handle(Event              &) override;
            bool handle_drop(DropEvent     &) override;
            bool handle_drag(DragEvent     &) override;
            bool handle_mouse(MouseEvent   &) override;
            bool handle_wheel(WheelEvent   &) override;
            bool handle_motion(MotionEvent &) override;
            bool handle_keyboard(KeyEvent  &) override;
            bool handle_textinput(TextInputEvent     &) override;
            bool handle_textediting(TextEditingEvent &) override;
            /**
             * @brief Set the delegate widget of this widget
             * 
             */
            void set_delegate(Widget *w){
                _delegate = w;
            }
            /**
             * @brief Get the delegate widget of this widget
             * 
             * @return Widget* 
             */
            Widget *delegate() const{
                return _delegate;
            }
        private:
            Widget *_delegate = nullptr;//< Delegate widget
    };
    /**
     * @brief A Interface for manage widget
     * 
     */
    class BTKAPI Group:public Container{
        public:
            Group(){
                attr.focus_policy = FocusPolicy::Mouse;
            }
            ~Group() = default;
        public:
            void draw(Renderer &,Uint32     ) override;

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

            //FIXME:Should we add a method to tell Group to invalidate the cache
            // like Group::invalidate_cache()?
            void invalidate();
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
            
            void draw(Renderer &,Uint32) override;
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
    class BTKAPI ListView:public Group{

    };
    class BTKAPI TreeView:public Group{
        
    };
    /**
     * @brief Lay Widget
     * 
     */
    class BTKAPI TabWidget:public Group{
        public:
            TabWidget();
            ~TabWidget();

            BTK_SHADOW_CONTAINER_METHOD();

            Group &insert_tab(u8string_view txt,long where);
        private:
            Signal<void(int change_to)> signal_tab_changed;
            long current_index = 0;
    };
    /**
     * @brief a group of widgets,but each time only one widget can be shown
     * @note Index start from 0(<0 is invalid,means no widget will be shown)
     */
    class BTKAPI StackedWidget:public Group{
        public:
            StackedWidget();
            ~StackedWidget();

            Widget *current_widget() const{
                return _current_widget;
            }
            Sint32  current_index() const{
                return _current_index;
            }
            /**
             * @brief Get the widget index
             * 
             * @param w 
             * @return Sint32 
             */
            Sint32 index_of(Widget *w) const{
                if(w == nullptr){
                    return -1;
                }
                void *p = w->userdata(BTK_STACKEDWIDGET_INDEX);
                if(p == nullptr){
                    return -1;
                }
                return LoadPodInPointer<Sint32>(p);
            }

            void set_current_widget(Widget *w);
            void set_current_widget(Sint32 index);

            using Container::add;
            /**
             * @brief Add a widget to the stack,The first widget added index is 0
             * 
             * @param w 
             * @return true 
             * @return false 
             */
            bool add(Widget *w) override;
            bool detach(Widget *w) override;
            void move_widget(Widget *w,long index) override;

            //Exposed signals
            auto signal_current_changed() -> Signal<void(Sint32 )> &{
                return _signal_current_changed;
            }
            auto signal_widget_removed() -> Signal<void(Sint32 )> &{
                return _signal_widget_removed;
            }
        private:
            Sint32 proc_index(Sint32) const;

            Sint32  _current_index = -1;//< The index of current widget
            Widget *_current_widget = nullptr;
            bool _allow_invalid_index = false;//< Use invalid index to show nothing

            //Signals
            Signal<void(Sint32 change_to)> _signal_current_changed;
            Signal<void(Sint32 index)> _signal_widget_removed;
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

            void draw(Renderer &,Uint32) override;
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
            void draw(Renderer &,Uint32) override final;
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
