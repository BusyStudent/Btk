#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include <string_view>
#include <functional>
#include <string>
#include "signal/signal.hpp"
#include "defs.hpp"
namespace Btk{
    struct WindowImpl;
    class Surface;
    class Widget;
    class BTKAPI Window:public HasSlots{
        public:
            //Signals
            typedef Signal<bool()> SignalClose;
            typedef Signal<void(int w,int h)> SignalResize;
            typedef Signal<void(std::string_view)> SignalDropFile;
            //Current window for add widgets
            static thread_local Window *Current;
        public:
            Window():pimpl(nullptr){};
            Window(const Window &) = delete;
            Window(Window &&win){
                pimpl = win.pimpl;
                win.pimpl = nullptr;
            };
            Window(std::string_view title,int w,int h);
            //Get impl
            inline WindowImpl *impl() const noexcept{
                return pimpl;
            }
            //add widget
            bool add(Widget *ptr);
            template<class T,class ...Args>
            T &add(Args &&...args){
                T *ptr = new T(std::forward<Args>(args)...);
                add(ptr);
                return *ptr;
            }
            //update widgets postions
            void update();
            //multi threading
            void lock();
            void unlock();
            //Show window and set Widget postions
            void done();
            //Move window position
            void move(int x,int y);
            void show();
            void draw();
            void close();//try close window
            void present();
            bool mainloop();
            //Window exists
            bool exists() const;
            //Get window surface
            Surface surface();
            //Set window title
            void set_title(std::string_view title);
            //Set window Icon
            void set_icon(std::string_view file);
            void set_icon(const Surface &surf);
            //Set window resizeable
            void set_resizeable(bool val = true);
            //Set Callbacks
            template<class T>
            Connection on_close(T &&callable){
                return sig_close().connect(std::forward<T>(callable));
            }
            template<class T>
            Connection on_resize(T &&callable){
                return sig_resize().connect(std::forward<T>(callable));
            }
            template<class T>
            Connection on_dropfile(T &&callable){
                return sig_dropfile().connect(std::forward<T>(callable));
            }
            //Connect Signals
            SignalClose&    sig_close();
            SignalResize&   sig_resize();
            SignalDropFile& sig_dropfile();
            //Set window cursor
            void set_cursor();//reset to default
            void set_cursor(const Surface &surf,int hot_x = 0,int hot_y = 0);
            //Start add widgets
            void make_current() noexcept{
                Window::Current = this;
            }
            //Get information
            int w() const noexcept;//get w
            int h() const noexcept;//get h
        private:
            WindowImpl *pimpl;
    };
}
#endif // _BTK_WINDOW_HPP_
