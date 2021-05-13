#if !defined(_BTK_APPLICATION_HPP_)
#define _BTK_APPLICATION_HPP_
#include "defs.hpp"
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
        private:
            ApplicationImpl *app;
    };
}

#endif // _BTK_APPLICATION_HPP_
