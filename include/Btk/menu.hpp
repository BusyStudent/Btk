#if !defined(_BTK_MENU_HPP_)
#define _BTK_MENU_HPP_
#include "defs.hpp"
#include "widget.hpp"
#include "container.hpp"

namespace Btk{
    //TODO FInished the menu
    class BTKAPI Menu:public Group{
        public:
            Menu();
            ~Menu();

            void set_parent(Widget*) override;
        private:
            bool _use_native = true;
    };
}

#endif // _BTK_MENU_HPP_
