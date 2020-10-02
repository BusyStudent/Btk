#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include <string_view>
#include <functional>
#include <string>
#include "defs.hpp"
namespace Btk{
    struct WindowImpl;
    class BTKAPI Window{
        public:
            //callback functions
            typedef std::function<bool()> OnCloseFunc;
            typedef std::function<void(std::string_view)> OnDropFileFunc;
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
            //Move window position
            void move(int x,int y);

            void show();
            void draw();
            void present();
            bool mainloop();
            //Set window title
            void set_title(std::string_view title);
            //Set window Icon
            void set_icon(std::string_view file);
            //Set callback
            void on_close(const OnCloseFunc &);
            void on_dropfile(const OnDropFileFunc &);
        private:
            WindowImpl *pimpl;
    };
}
#endif // _BTK_WINDOW_HPP_
