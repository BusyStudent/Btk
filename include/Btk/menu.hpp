#if !defined(_BTK_MENU_HPP_)
#define _BTK_MENU_HPP_
#include "defs.hpp"
#include "widget.hpp"
#include "container.hpp"

namespace Btk{
    class Action;
    /**
     * @brief Menu Interface
     * 
     */
    class BTKAPI Menu{
        public:
            Menu();
            ~Menu();

            Menu *add_menu();

        private:
            std::list<Action*> actions;
    };
    /**
     * @brief MenuBar
     * 
     */
    class BTKAPI MenuBar:public Group,public Menu{
        public:
            MenuBar();
            ~MenuBar();

            void draw(Renderer &) override;
            void set_parent(Widget *parent) override;
        private:
            bool native_menu = false;
    };
    /**
     * @brief PopupMenu
     * 
     */
    class BTKAPI PopupMenu:public Group,public Menu{
        public:
            PopupMenu();
            ~PopupMenu();
        private:
            WindowImpl *_window = nullptr;//< The window of the popup menu
            WindowImpl *_parent = nullptr;//< The parent window of the popup menu
    };
    /**
     * @brief Action,better to construct at heap
     * 
     */
    class BTKAPI Action:public HasSlots{
        public:
            Action() = default;
            ~Action() = default;

            //Expose signal
            Signal<void()> &signal_triggered() noexcept{
                return _signal_triggered;
            }
            Signal<void()> &signal_hovered() noexcept{
                return _signal_hovered;
            }
        private:
            u8string value;
            Signal<void()> _signal_triggered;
            Signal<void()> _signal_hovered;
    };
}

#endif // _BTK_MENU_HPP_
