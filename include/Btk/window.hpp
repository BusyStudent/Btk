#if !defined(_BTK_WINDOW_HPP_)
#define _BTK_WINDOW_HPP_
#include <string_view>
#include <string>

#include "defs.hpp"
namespace Btk{
    struct WindowImpl;
    class BTKAPI Window{
        public:
            Window():pimpl(nullptr){};
            Window(std::string_view title,int w,int h);
            ~Window();
            //Get impl
            inline WindowImpl *impl() const noexcept{
                return pimpl;
            }
            void show();
            void present();
            bool mainloop();
        private:
            WindowImpl *pimpl;
    };
}
#endif // _BTK_WINDOW_HPP_
