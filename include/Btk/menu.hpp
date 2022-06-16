#if !defined(_BTK_MENU_HPP_)
#define _BTK_MENU_HPP_
#include "defs.hpp"
#include "widget.hpp"
#include "window.hpp"
#include "container.hpp"

#define BTK_ACTION_OF_WIDGET "_action_"

namespace Btk{
    //TODO Still need to be improved
    class Action;
    /**
     * @brief Menu Interface
     * 
     */
    class BTKAPI Menu{
        public:
            Menu();
            ~Menu();
            /**
             * @brief Add a sub menu
             * 
             * @return Menu* 
             */
            Menu *add_menu(u8string_view name);
            /**
             * @brief Add a menu item
             * 
             * @param action 
             */
            void  add_action(Action *action);
            void  remove_action(Action *action);

            //Expose signals
            [[nodiscard]]
            auto signal_add_action() -> Signal<void(Action *act)> &{
                return _signal_add_action;
            }
            [[nodiscard]]
            auto signal_remove_action() -> Signal<void(Action *act)> &{
                return _signal_remove_action;
            }
        private:
            Signal<void(Action *act)> _signal_add_action;
            Signal<void(Action *act)> _signal_remove_action;

            Menu *_parent = nullptr;
            bool  _popup = false;
        protected:
            std::list<Action*> actions;
        friend class PopupMenu;
    };
    /**
     * @brief MenuBar
     * 
     */
    class BTKAPI MenuBar:public Group,public Menu{
        public:
            MenuBar();
            ~MenuBar();

            void draw(Renderer &,Uint32) override;
            void set_rect(const Rect &r) override;
            void set_parent(Widget *parent) override;
        private:
            void on_add_action(Action *act);
            void on_remove_action(Action *act);
            /**
             * @brief Update action's widget
             * 
             */
            void update_action();

            bool native_menu = false;
    };
    /**
     * @brief PopupMenu
     * 
     */
    class BTKAPI PopupMenu:public HasSlots,public Menu{
        public:
            PopupMenu();
            ~PopupMenu();

            /**
             * @brief Exec the popup menu,and it will block the current thread
             * @param x The menubar x
             * @param y The menubar y
             * @note It is safe to run at main thread(if you has set parent)
             */
            void run(int x = -1,int y = -1);
            void set_parent(Window &p){
                _parent = &p;
            }
            void set_parent(Window *p){
                _parent = p;
            }
        private:
            Window *_window = nullptr;//< The window of the popup menu
            Window *_parent = nullptr;//< The parent window of the popup menu
        friend class Menu;
    };
    /**
     * @brief Action,better to construct at heap
     * 
     */
    class BTKAPI Action:public HasSlots{
        public:
            Action() = default;
            Action(const Action &) = delete;
            ~Action() = default;

            Action(u8string_view text){
                _value = text;
            }

            //Expose signal
            Signal<void()> &signal_triggered() noexcept{
                return _signal_triggered;
            }
            Signal<void()> &signal_hovered() noexcept{
                return _signal_hovered;
            }
            Signal<void()> &signal_value_changed() noexcept{
                return _signal_value_changed;
            }

            void set_text(u8string_view text){
                _value = text;
                _signal_value_changed();
            }
            void set_icon(PixBufRef i){
                _icon = i;
                _signal_value_changed();
            }
            void set_menu(Menu *m){
                _menu = m;
                _signal_value_changed();
            }
            void set_widget(Widget *w){
                _widget = w;
                _signal_value_changed();
            }
            //Get the text
            u8string_view text() const{
                return _value;
            }
            PixBufRef icon() const{
                return _icon;
            }
            Menu *menu() const{
                return _menu;
            }
            Widget *widget() const{
                return _widget;
            }

            //Utils for menu
            void on_triggered(){
                _signal_triggered();
            }
            void on_hovered(){
                _signal_hovered();
            }
        private:
            u8string _value;
            PixBuf   _icon;
            Signal<void()> _signal_triggered;
            Signal<void()> _signal_hovered;
            Signal<void()> _signal_value_changed; 

            Widget *_widget = {};//< The widget associated with this action
            Menu *_menu = {};//< The menu associated with this action

    };
}

#endif // _BTK_MENU_HPP_
