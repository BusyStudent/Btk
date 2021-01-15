#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include <string_view>
#include <functional>
#include <string>
#include "signal/signal.hpp"
#include "rect.hpp"
#include "defs.hpp"
namespace Btk{
    struct WindowImpl;
    class PixBuf;
    class Widget;
    class Event;
    class Font;
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
            typedef Signal<void(std::string_view)> SignalDropFile;
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
            Window(std::string_view title,int w,int h);
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
                T *ptr = new T(*this,std::forward<Args>(args)...);
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
            void set_title(std::string_view title);
            /**
             * @brief Set the icon 
             * 
             * @param file The image file name
             */
            void set_icon(std::string_view file);
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
             * @brief Set the background transparent
             * 
             * @note Some platform unsupport it
             * @param val The transparent value
             */
            void set_transparent(float value);
            //Set Callbacks
            template<class ...T>
            Connection on_close(T &&...args){
                return sig_close().connect(std::forward<T>(args)...);
            }
            template<class ...T>
            Connection on_resize(T &&...args){
                return sig_resize().connect(std::forward<T>(args)...);
            }
            template<class ...T>
            Connection on_dropfile(T &&...args){
                return sig_dropfile().connect(std::forward<T>(args)...);
            }
            //Connect Signals
            SignalClose&    sig_close();
            SignalEvent&    sig_event();
            SignalResize&   sig_resize();
            SignalDropFile& sig_dropfile();
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
        private:
            WindowImpl *pimpl;
            Uint32 winid;
    };
}
#endif // _BTK_WINDOW_HPP_
