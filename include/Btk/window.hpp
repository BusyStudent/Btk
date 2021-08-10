#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include "string.hpp"
#include "signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
#include <cstdio>
namespace Btk{
    class WindowImpl;
    class Container;
    class PixBuf;
    class Widget;
    class Event;
    class Font;
    enum class WindowFlags:Uint32{
        None        = 0,
        OpenGL      = 1 << 0,
        Vulkan      = 1 << 1,
        SkipTaskBar = 1 << 2,
    };
    /**
     * @brief Class for describe native window
     * 
     */
    class NativeWindow;
    /**
     * @brief A Basic Window 
     * 
     */
    class BTKAPI Window:public HasSlots{
        public:
            //Signals
            typedef Signal<bool()> SignalClose;
            typedef Signal<bool(Event&)> SignalEvent;
            typedef Signal<void(int w,int h)> SignalResize;
            typedef Signal<void(u8string_view)> SignalDropFile;
            //Flags
            using Flags = WindowFlags;
        public:
            Window():pimpl(nullptr){};
            Window(const Window &) = delete;
            Window(Window &&win){
                pimpl = win.pimpl;
                win.pimpl = nullptr;
            };
            /**
             * @brief Construct a new Window object
             * 
             * @param title The window title
             * @param w The window width
             * @param h Thw window height
             */
            //Window(std::string_view title,int w,int h);
            explicit Window(u8string_view title,int w,int h,Flags f = Flags::None);
            /**
             * @brief Construct a new Window object by native window handle
             * 
             * @param native_handle (reinterpret_cast<const NativeWindow*>(your_handle))
             */
            explicit Window(const NativeWindow *native_handle);
            /**
             * @brief Get window impl
             * 
             * @return WindowImpl* 
             */
            inline WindowImpl *impl() const noexcept{
                return pimpl;
            }
            /**
             * @brief Add widget to window
             * 
             * @param ptr The widget pointer(It can be nullptr)
             * @return true The ptr is not nullptr
             * @return false The ptr is nullptr
             */
            bool add(Widget *ptr);
            /**
             * @brief A helper template to add widget
             * 
             * @tparam T The widget type
             * @tparam Args The args to want to pass to construc it
             * @param args  The args
             * @return T& The widget reference
             */
            template<class T,class ...Args>
            T &add(Args &&...args){
                T *ptr = new T(std::forward<Args>(args)...);
                add(ptr);
                return *ptr;
            }
            /**
             * @brief Update widgets position
             * 
             */
            void update();
            /**
             * @brief Lock the window before access it
             * 
             */
            void lock();
            /**
             * @brief Unlock thw window
             * 
             */
            void unlock();
            /**
             * @brief Show window and set Widget postions
             * 
             */
            void done();
            /**
             * @brief Move the window
             * 
             * @param x The new x
             * @param y The new y
             */
            void move(int x,int y);
            /**
             * @brief Make the window visiable
             * 
             */
            void show();
            /**
             * @brief Redraw the window
             * 
             * @note Is is safe to call in multithreading without lock it
             */
            void draw();
            /**
             * @brief Send a close request
             * 
             * @note Is is safe to call in multithreading without lock it
             */
            void close();
            /**
             * @brief Enter the main event loop
             * 
             * @return true The event loop finished normally
             * @return false Double call
             */
            bool mainloop();
            /**
             * @brief Check the window is exist
             * 
             * @return true The window is exists
             * @return false The window is not exists
             */
            bool exists() const;
            /**
             * @brief Get window's pixbuf
             * 
             * @return PixBuf 
             */
            PixBuf pixbuf();
            /**
             * @brief Set the title 
             * 
             * @param title The new title
             */
            void set_title(u8string_view title);
            /**
             * @brief Set the icon 
             * 
             * @param file The image file name
             */
            void set_icon(u8string_view file);
            /**
             * @brief Set the icon 
             * 
             * @param pixbuf The image pixbuf
             */
            void set_icon(const PixBuf &pixbuf);
            /**
             * @brief Set the fullscreen
             * 
             * @param val The fullscreen flags
             */
            void set_fullscreen(bool val = true);
            /**
             * @brief Set the resizeable 
             * 
             * @param val The resizeable flags
             */
            void set_resizeable(bool val = true);
            /**
             * @brief Set the boardered 
             * 
             * @param val The boarder
             */
            void set_boardered(bool val = true);
            /**
             * @brief Set the background transparent
             * 
             * @note Some platform unsupport it
             * @param val The transparent value
             */
            void set_transparent(float value);
            //Set Callbacks
            template<class ...T>
            Connection on_close(T &&...args){
                return signal_close().connect(std::forward<T>(args)...);
            }
            template<class ...T>
            Connection on_resize(T &&...args){
                return signal_resize().connect(std::forward<T>(args)...);
            }
            template<class ...T>
            Connection on_dropfile(T &&...args){
                return signal_dropfile().connect(std::forward<T>(args)...);
            }
            //Connect Signals
            SignalClose&    signal_close();
            SignalEvent&    signal_event();
            SignalResize&   signal_resize();
            SignalDropFile& signal_dropfile();
            /**
             * @brief Set the cursor to default
             * 
             */
            void set_cursor();
            /**
             * @brief Set the cursor 
             * 
             * @param pixbuf The image pixelbuf
             * @param hot_x The cursor's x
             * @param hot_y The cursor's y
             */
            void set_cursor(const PixBuf &pixbuf,int hot_x = 0,int hot_y = 0);
            //Get information
            int w() const noexcept;//get w
            int h() const noexcept;//get h
            Font font() const;//get font
            /**
             * @brief Dump the widget tree
             * 
             */
            void dump_tree(FILE *output = stderr) const;
            Container &container() const;
        private:
            WindowImpl *pimpl;
            Uint32 winid;
    };
    /**
     * @brief Get the Screen Size object
     * @param display The display index(0 on default)
     * @return BTKAPI 
     */
    BTKAPI Size GetScreenSize(int display = 0);
    inline WindowFlags operator +(WindowFlags a,WindowFlags b){
        return WindowFlags(Uint32(a) | Uint32(b));
    }
    inline WindowFlags operator |(WindowFlags a,WindowFlags b){
        return WindowFlags(Uint32(a) | Uint32(b));
    }
    inline WindowFlags operator &(WindowFlags a,WindowFlags b){
        return WindowFlags(Uint32(a) & Uint32(b));
    }
}
#endif // _BTK_WINDOW_HPP_
