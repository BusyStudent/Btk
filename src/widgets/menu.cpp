#include "../build.hpp"

#include <Btk/detail/window.hpp>
#include <Btk/signal/bind.hpp>
#include <Btk/exception.hpp>
#include <Btk/button.hpp>
#include <Btk/layout.hpp>
#include <Btk/menu.hpp>
#include <Btk/Btk.hpp>
#include <algorithm>
namespace Btk{
    void Menu::add_action(Action *act){
        //Assert action is not in the menu
        BTK_ASSERT(std::find(actions.begin(), actions.end(), act) == actions.end());

        actions.push_back(act);
        _signal_add_action(act);
    }
    void Menu::remove_action(Action *act){
        auto it = std::find(actions.begin(), actions.end(), act);
        if(it != actions.end()){
            actions.erase(it);
            _signal_remove_action(act);
        }
    }
    Menu::Menu(){

    }
    Menu::~Menu(){
        for(auto &action: actions){
            delete action;
        }
    }
    Menu *Menu::add_menu(u8string_view name){
        //Sub menu
        Action *action = new Action(name);
        PopupMenu *menu = new PopupMenu();

        //Bind menu to action
        action->set_menu(menu);
        action->signal_triggered().connect([action,menu,this](){
            //Get the widget according to the action
            Widget *w = action->widget();
            BTK_ASSERT(w);
            //Get the parent window of the widget
            Widget  *_win = w->root();
            BTK_ASSERT(_win->is_window());
            //Cast to window
            WindowImpl *win = dynamic_cast<WindowImpl*>(_win);
            //Translate the widget postion into global
            auto win_pos = win->position();
            auto r_pos = win_pos.translate(
                w->x(),
                w->y() + w->h()
            );
            //Parent is a popup menu
            if(_popup){
                //Add X with W;
                r_pos.x += win->size().w;
                r_pos.y -= static_cast<Container*>(
                    win->index_widget(0)
                )->index_widget(-1)->h();
            }
            //Popup the menu
            menu->set_parent(win);
            menu->run(r_pos.x,r_pos.y);
            menu->set_parent(nullptr);
            //Done
        });
        menu->Menu::_parent = this;
        menu->Menu::_popup = true;
        add_action(action);
        action->on_destroy([menu](){
            delete menu;
        });
        return menu;
    }


    MenuBar::MenuBar(){
        signal_add_action().connect(&MenuBar::on_add_action,this);
        signal_remove_action().connect(&MenuBar::on_remove_action,this);
    }
    MenuBar::~MenuBar(){

    }
    void MenuBar::set_parent(Widget *w){
        Group::set_parent(w);
        if(w != nullptr){
            //Set the position of the menu bar
            set_rectangle(0,0,parent()->w(),theme().menubar_height);
        }
    }
    void MenuBar::set_rect(const Rect &r){
        auto new_r = r;
        new_r.h = theme().menubar_height;

        Group::set_rect(new_r);
        //Set All children's x,y
        update_action();
    }
    void MenuBar::on_add_action(Action *act){
        //Add the action to the menu bar
        auto btn = new Button(act->text());
        act->set_widget(btn);
        //How to manage the con
        act->signal_value_changed().connect([this,btn,act](){
            btn->set_text(act->text());
            update_action();
        });
        btn->signal_clicked().connect(
            &Action::on_triggered,act
        );
        btn->set_draw_border(false);
        add(btn);
        update_action();
    }
    void MenuBar::on_remove_action(Action *act){
        //Remove the action from the menu bar
        auto btn = act->widget();
        remove(btn);
        act->set_widget(nullptr);
        update_action();
    }
    //TODO: add chache
    void MenuBar::update_action(){
        if(w() == 0 or h() == 0){
            //Invalid 
            return;
        }
        auto &ft = font();
        //Pack widgets
        int x = rect.x,y = rect.y;
        Widget *prev = nullptr;
        
        for(auto act:actions){
            //Mesure the text
            auto txt = act->text();
            auto [w,h] = ft.size(txt);
            //Reset rectange of the action's widget
            w += 8;//< TODO:add this pad into theme
            act->widget()->set_rect(
                x,
                y,
                w,
                rect.h
            );
            x += w;
        }
        // #ifndef NDEBUG
        // dump_tree();
        // #endif
        redraw();
    }
    void MenuBar::draw(Renderer &r,Uint32 t){
        r.draw_box(rectangle(),theme().active.button);
        Group::draw(r,t);
        //Draw the menu bar
        r.draw_line(x(),y()+h()-1,x()+w(),y()+h()-1,theme().active.border);
    }

    //PopupMenu
    PopupMenu::PopupMenu(){

    }
    PopupMenu::~PopupMenu(){

    }
    void PopupMenu::run(int x,int y){
        //Check has parent and is main thread
        BTK_ASSERT(_parent != nullptr);
        BTK_ASSERT(IsMainThread());

        if(actions.empty()){
            return;
        }
        if(x < 0 and y < 0){
            SDL_GetGlobalMouseState(&x,&y);
        }
        _parent->set_modal(true);
        //Select All action ,generate button
        auto &ft = _parent->font();
        int h = _parent->theme().menubar_height * actions.size();
        int w = 0;
        for(auto act:actions){
            auto [w1,h1] = ft.size(act->text());
            w1 += 8;//< TODO:add this pad into theme
            w = max(w,int(std::ceil(w1)));
        }
        //Create the window
        //FIXME: PopupMenu won't receive Keyboard focus lost event
        //FIXME: We must use WindowFlags::PopupMenu
        //FIXME: Bug if submenu has a submenu
        _window = CreateWindow(
            {},
            w,
            h,
            WindowFlags::Borderless
        );
        //Add widgets
        auto &lay = _window->add<VBoxLayout>();
        
        for(auto act:actions){
            auto btn = new Button(act->text());
            act->set_widget(btn);
            btn->signal_clicked().connect(
                &Action::on_triggered,act
            );
            btn->set_draw_border(false);
            lay.add(btn);
        }

        auto check_belong_child = [&](WindowImpl *win,Menu *menu) -> bool{
            auto do_it = [&](auto &&self,WindowImpl *win,Menu *menu) -> bool{
                PopupMenu *m = static_cast<PopupMenu*>(menu);
                if(m->_window == win){
                    return true;
                }

                for(auto &sub:m->actions){
                    if(sub->menu() == nullptr){
                        continue;
                    }
                    if(not sub->menu()->_popup){
                        continue;
                    }
                    if(self(self,win,sub->menu())){
                        return true;
                    }
                }
                return false;
            };
            return do_it(do_it,win,menu);
        };

        _window->signal_keyboard_lost_focus().connect([&](){
            WindowImpl *focus_win = GetMouseFocus();
            //Check the focus_win belongs to child or self
            if(check_belong_child(focus_win,this)){
                return;
            }
            //Check the focus_win belongs to parent
            Menu *parent = Menu::_parent;
            int n = 1;//Interrup self
            while(parent != nullptr){
                if(not parent->_popup){
                    break;
                }
                n += 1;
                PopupMenu *cur = static_cast<PopupMenu*>(parent);
                if(cur->_window == focus_win){
                    //Is parent window got focus
                    break;
                }
                parent = parent->Menu::_parent;
            }
            while(n != 0){
                n -= 1;
                InterruptLoop();
            }
        });

        _window->move(x,y);
        _window->show();

        //Wait for the window to be destroyed
        while(WaitEvent()){

        }

        //Reset action widget
        for(auto act:actions){
            act->set_widget(nullptr);
        }
        _parent->set_modal(false);
        _window->close();
    }
}