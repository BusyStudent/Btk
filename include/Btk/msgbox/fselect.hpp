#if !defined(_BTK_FSELECT_HPP_)
#define _BTK_FSELECT_HPP_
//File select box
#include <string>
#include "../defs.hpp"
#include "../signal/signal.hpp"
namespace Btk{
    //The impl of the box
    struct FSelectBoxImpl;
    /**
     * @brief A Dialog of file selecting
     * 
     */
    class BTKAPI FSelectBox{
        public:
            typedef Signal<void(std::string_view txt)> SignalAsync;
        public:
            FSelectBox(std::string_view title = std::string_view());
            ~FSelectBox();
            void show();
            /**
             * @brief Get the async signal,
             *  It will be emited after closeing the box
             * 
             * @return SignalAsync& 
             */
            SignalAsync &sig_async();
            /**
             * @brief Allow to select multi files
             * 
             * @param val True or false
             */
            void set_multi(bool val = true);
            /**
             * @brief Save file dialog
             * 
             * @param val True to enable the attribute
             */
            void set_save(bool val = true);
        private:
            FSelectBoxImpl *pimpl;
    };
};


#endif // _BTK_FSELECT_HPP_