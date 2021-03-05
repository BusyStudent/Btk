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
                Info,//Information
                Warn,//Warning
                Error,//Error
            };
            /**
             * @brief Construct a new Message Box object
             * 
             * @param title The MessageBox title
             * @param txt The messageBox message
             * @param flags The messagebox flag
             */
            MessageBox(std::string_view title = {},
                       std::string_view txt = {},
                       Flag flags = Info);
            MessageBox(const MessageBox &);
            ~MessageBox();

            SignalAsync &sig_async();
            void set_title(std::string_view title);
            void set_message(std::string_view message);
            void set_flag(Flag flag);
            bool show(bool focus_async = false);
        private:
            MessageBoxImpl *pimpl;
    };
}


#endif // _BTK_MESSAGEBOX_HPP_
