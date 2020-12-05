#if !defined(_BTK_MESSAGEBOX_HPP_)
#define _BTK_MESSAGEBOX_HPP_
#include "../window.hpp"
#include "../button.hpp"
#include "../lable.hpp"

namespace Btk{
    class MessageBox{
        MessageBox(std::string_view title,std::string_view msg);
        Window *win;
        Lable  &lable;
        Button &button;
        void show();
    };
};


#endif // _BTK_MESSAGEBOX_HPP_
