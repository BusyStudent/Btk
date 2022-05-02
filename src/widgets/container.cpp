#include "../build.hpp"

#include <Btk/container.hpp>
#include <Btk/exception.hpp>
#include <Btk/render.hpp>

#include <Btk/event.hpp>
namespace Btk{
    GroupBox::GroupBox(){
        background_color = theme().active.background;
        borader_color = theme().active.border;
    }
    GroupBox::~GroupBox() = default;
    void GroupBox::draw(Renderer &render,Uint32 t){
        if(draw_background){
            render.draw_box(rectangle(),background_color);
        }
        Group::draw(render,t);
        if(draw_boarder){
            render.draw_rect(rectangle(),borader_color);
        }
    }
    //DelegateWidget --begin
    void DelegateWidget::set_parent(Widget *w){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->set_parent(w);
    }
    void DelegateWidget::set_rect(const Rect &r){
        Widget::set_rect(r);
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->set_rect(r);
    }
    void DelegateWidget::draw(Renderer &p,Uint32    ticks){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->draw(p,ticks);
    }
    bool DelegateWidget::handle(Event              &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle(event);
    }
    bool DelegateWidget::handle_drop(DropEvent     &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_drop(event);
    }
    bool DelegateWidget::handle_drag(DragEvent     &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_drag(event);
    }
    bool DelegateWidget::handle_mouse(MouseEvent   &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_mouse(event);
    }
    bool DelegateWidget::handle_wheel(WheelEvent   &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_wheel(event);
    }
    bool DelegateWidget::handle_motion(MotionEvent &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_motion(event);
    }
    bool DelegateWidget::handle_keyboard(KeyEvent  &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_keyboard(event);
    }
    bool DelegateWidget::handle_textinput(TextInputEvent     &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_textinput(event);
    }
    bool DelegateWidget::handle_textediting(TextEditingEvent &event){
        BTK_ASSERT(_delegate != nullptr);
        return _delegate->handle_textediting(event);
    }
    //DelegateWidget --end

    //StackedWidget
    StackedWidget::StackedWidget(){
        //
    }
    StackedWidget::~StackedWidget(){
        //
    }
    // void StackedWidget::set_rect(const Rect &r){
    //     Widget::set_rect(r);
    //     if(_current_widget != nullptr){
    //         _current_widget->set_rect(r);
    //     }
    // }
    // //Send all event to the top widget
    // bool StackedWidget::handle(Event &e){
    //     if(Widget::handle(e)){
    //         return true;
    //     }
    //     //FIXME: did we send hide / show event to the widget?
    //     if(_current_widget != nullptr){
    //         return _current_widget->handle(e);
    //     }
    //     return false;
    // }
    // void StackedWidget::draw(Renderer &r,Uint32 t){
    //     if(_current_widget != nullptr){
    //         _current_widget->draw(r,t);
    //     }
    // }
    Sint32 StackedWidget::proc_index(Sint32 idx) const{
        if(idx < 0){
            return -1;//< means no widget will be shown
        }
        if(idx >= get_childrens().size()){
            if(not _allow_invalid_index){
                throwRuntimeError("unknown widget in container");
            }
            return -1;//> means no widget will be shown
        }
        return idx;
    }
    bool StackedWidget::add(Widget *w){
        if(Group::add(w)){
            //Mark position
            void *v;
            StorePodInPointer<Sint32>(&v,get_childrens().size() - 1);
            w->set_userdata(BTK_STACKEDWIDGET_INDEX,v);
            //Hide default
            w->hide();
            return true;
        }
        return false;
    }
    bool StackedWidget::detach(Widget *w){
        if(not Group::detach(w)){
            return false;
        }
        //Remove 
        if(_current_widget == w){
            //Reset current widget
            set_current_widget(nullptr);
        }
        if constexpr(sizeof(Sint32) > sizeof(void*)){
            //Cleanup pointer
            DeletePodInPointer<Sint32>(w->userdata(BTK_STACKEDWIDGET_INDEX));
        }
        w->set_userdata(BTK_STACKEDWIDGET_INDEX,nullptr);
        return true;
    }
    void StackedWidget::move_widget(Widget *w,long){
        //Unimplemented yet
        BTK_UNIMPLEMENTED();
    }
    void StackedWidget::set_current_widget(Widget *w){
        Sint32 idx;
        if(w == _current_widget){
            return;
        }
        if(w != nullptr){
            idx = index_of(w);
            if(idx == -1){
                throwRuntimeError("unknown widget in container");
            }
        }
        else{
            idx = -1;
        }
        //Hide prev
        if(_current_widget != nullptr){
            _current_widget->hide();
        }
        //OK set the current widget
        _current_widget = w;
        _current_index = idx;

        w->show();

        _signal_current_changed.defer_emit(idx);

        redraw();
    }
    void StackedWidget::set_current_widget(Sint32 idx){
        //Nagative index is invalid
        idx = proc_index(idx);
        if(idx == _current_index){
            return;
        }
        Widget *new_w = index_widget(idx);
        if(_current_widget != nullptr){
            _current_widget->hide();
        }

        _current_widget = new_w;
        _current_index = idx;

        _current_widget->show();

        _signal_current_changed.defer_emit(idx);
        redraw();
    }
    //StackedWidget --end
}