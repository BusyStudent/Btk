#if !defined(_BTK_APPLICATION_HPP_)
#define _BTK_APPLICATION_HPP_
#include "defs.hpp"
#include "pixels.hpp"
#include "object.hpp"
#include "string.hpp"
namespace Btk{
    struct ApplicationImpl;
    /**
     * @brief Application
     * 
     */
    class BTKAPI Application:public HasSlots{
        public:
            using SignalQuit = Signal<void()>;
            using SignalClipboardUpdate = Signal<void()>;
        public:
            Application();
            Application(const Application &) = delete;
            ~Application();
            /**
             * @brief Show notify
             * 
             * @param title 
             * @param msg 
             * @return true 
             * @return false 
             */
            bool notify(u8string_view title = {},u8string_view msg = {});
            bool mainloop();
            int  run();
            /**
             * @brief Get the signal which will be emit 
             *        when the os asked the app to quit
             * 
             * @return SignalQuit& 
             */
            Signal<void()> &signal_quit();
            Signal<void()> &signal_clipboard_update();

            void set_clipboard(const PixBuf &image);
            void set_clipboard(const u8string &text);
        private:
            ApplicationImpl *app;
    };
}

#endif // _BTK_APPLICATION_HPP_
