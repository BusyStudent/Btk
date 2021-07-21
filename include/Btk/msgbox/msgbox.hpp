#if !defined(_BTK_MESSAGEBOX_HPP_)
#define _BTK_MESSAGEBOX_HPP_
#include "../defs.hpp"
#include "../signal.hpp"
#include "../window.hpp"
#include <string_view>
namespace Btk{
    struct MessageBoxImpl;
    class BTKAPI MessageBox{
        public:
            typedef Signal<void()> SignalAsync;
            enum Flag:int{
                Info = 0,//Information
                Warn = 1,//Warning
                Error = 2,//Error
            };
            /**
             * @brief Construct a new Message Box object
             * 
             * @param title The MessageBox title
             * @param txt The messageBox message
             * @param flags The messagebox flag
             */
            MessageBox(u8string_view title = {},
                       u8string_view txt = {},
                       Flag flags = Info);
            MessageBox(const MessageBox &);
            ~MessageBox();

            SignalAsync &signal_async();
            void set_title(u8string_view title);
            void set_message(u8string_view message);
            void set_flag(Flag flag);
            bool show(bool focus_async = false);
        private:
            MessageBoxImpl *pimpl;
    };
}


#endif // _BTK_MESSAGEBOX_HPP_
