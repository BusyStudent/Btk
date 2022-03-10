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
            void add_menu(Menu *menu);
        private:
            Color boarder_color;

            bool _use_native = true;
            bool _draw_boarder = true;
            bool _draw_shadow = true;
    };
    class BTKAPI PopupMenu:public Group{

    };
    class BTKAPI Action:public HasSlots{
        private:
            u8string value;
    };
}

#endif // _BTK_MENU_HPP_
