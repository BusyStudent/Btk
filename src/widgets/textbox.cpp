#include "../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/detail/window.hpp>
#include <Btk/detail/input.hpp>
#include <Btk/detail/scope.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/textbox.hpp>
#include <Btk/window.hpp>
#include <Btk/cursor.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>

#include <algorithm>


namespace Btk{
    using utf8::unchecked::utf8to16;
    using utf8::unchecked::utf16to8;
    //A inserter
    struct TextBoxInserter{
        TextBoxInserter(TextBox &t,std::u16string::iterator i):tb(t),cur(i){};
        TextBox &tb;//TextBox
        std::u16string::iterator cur;//Current iter
        TextBoxInserter &operator *(){
            return *this;
        }
        TextBoxInserter &operator ++(int){
            return *this;
        }
        TextBoxInserter &operator ++(){
            return *this;
        }
        //insert ch
        void operator =(char16_t ch){
            cur = tb.tb_text.insert(cur,ch);
            ++cur;
        }
    };
    bool TextBox::handle(Event &event){
        static Cursor cursor(SystemCursor::Ibeam);
        if(Widget::handle(event)){
            return true;
        }
        switch(event.type()){
            case Event::TakeFocus:{
                //Enable Text input
                event.accept();
                has_focus = true;
                show_line = true;
                timer.start();
                SetTextInputRect(&rect);
                StartTextInput();
                redraw();
                return true;
            }
            case Event::LostFocus:{
                //Stop Text input
                event.accept();
                has_focus = false;
                editing = false;
                timer.stop();
                StopTextInput();
                redraw();
                return true;
            }
            //Editing cursor
            case Event::Enter:{
                cursor.set();
                return true;
            }
            case Event::Leave:{
                cursor.reset();
                return true;
            }
        }
        return false;
    }
    TextBox::TextBox(){
        //Set theme()
        //theme() =   window()->theme();
        //tb_font = window()->font();

        //Get rendered text's h
        //ft_h = tb_font.height();
        
        //BTK_LOGINFO("TextBox %p's ft_h=%d",this,ft_h);
        //Set current position
        cur_txt = tb_text.begin();

        attr.focus = FocusPolicy::Mouse;

        //Set timer
        timer.set_interval(500);
        timer.set_callback(&TextBox::timeout,this);
    }
    TextBox::~TextBox(){
        timer.stop();
    }
    void TextBox::set_parent(Widget *w){
        Widget::set_parent(w);
        ptsize = theme().font.ptsize();
    }
    void TextBox::draw(Renderer &render){
        render.draw_box(rect,theme().active.background);
        
        //BTK_LOGINFO("RendererLineH %f,ft_h %d",render.font_height(),ft_h);
        ft_h = render.font_height();
        if(not tb_text.empty()){
            /*
            if(tb_buf.empty()){
                tb_buf = tb_font.render_blended(tb_text,theme().text_color);
            }
            texture = render.create_from(tb_buf);
            */
            //Render text
            render.save();
            render.intersest_scissor(rect);

            Rect txt_rect = rect;
            FSize text_size = render.text_size(tb_text);
            txt_rect.x += tb_boarder;
            //txt_rect.w = tb_buf->w;
            //txt_rect.h = tb_buf->h;
            txt_rect.y = CalculateYByAlign(rect,ft_h,Align::Center);

            render.begin_path();

            render.text_size(ptsize);
            render.text_align(TextAlign::Middle | TextAlign::Left);
            render.fill_color(theme().active.text);
            
            //plus 2 make it look better
            render.text(
                float(rect.x + tb_boarder),
                float(rect.y + float(rect.h) / 2) + 2,
                u16string_view(tb_text)
            );

            render.fill();


            if(has_focus and show_line){
                //draw a line for editing
                //calc the w
                int line_x;

                if(cur_txt == --tb_text.begin()){
                    //Just point at the rect.x
                    line_x = txt_rect.x;
                }
                else{
                    std::u16string view(tb_text.begin(),cur_txt + 1);
                    line_x = txt_rect.x + render.text_size(view).w;
                }
                
                render.draw_line(line_x,txt_rect.y,line_x,txt_rect.y + text_size.h,theme().active.text);
            }
            render.restore();
        }
        else if(has_focus and show_line){
            //render the line
            int line_y = CalculateYByAlign(rect,ft_h,Align::Center);
            render.draw_line(rect.x + tb_boarder,
                            line_y,
                            rect.x + tb_boarder,
                            line_y + ft_h,
                            theme().active.text
                            );
        }

        if(has_focus){
            render.draw_rounded_rect(rect,1,theme().active.highlight);
        }
        else{
            render.draw_rounded_rect(rect,1,theme().active.border);
        }
        render.restore();
    }
    bool TextBox::handle_keyboard(KeyEvent &event){
        if(editing){
            //Reject event if text is editing
            return event.reject();
        }
        event.accept();
        if(event.state == KeyEvent::Pressed){
            switch(event.keycode){
                case Keycode::Backspace:{
                    if(not tb_text.empty() and cur_txt != --tb_text.begin()){
                        
                        cur_txt = tb_text.erase(cur_txt);
                        --cur_txt;

                        #ifndef NDEBUG
                        std::string t;
                        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
                        BTK_LOGINFO("TextBox:text=%s",t.c_str());
                        #endif
                        //tb_buf = nullptr;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case Keycode::Left:{
                    if(cur_txt != --tb_text.begin()){
                        --cur_txt;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case Keycode::Right:{
                    if(cur_txt != --tb_text.end()){
                        ++cur_txt;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case Keycode::V:{
                    //Ctrl + V
                    if(event.has_kmod(Keymode::Ctrl)){
                        BTK_LOGINFO("Ctrl + V");
                        char *u8str = SDL_GetClipboardText();
                        if(u8str == nullptr){
                            return true;
                        }
                        SDLScopePtr ptr(u8str);
                        add_string(u8str);
                    } 
                    return true;
                }
                case Keycode::C:{
                    //Ctrl + C
                    if(event.has_kmod(Keymode::Ctrl)){
                        BTK_LOGINFO("Ctrl + C");
                        SDL_SetClipboardText(u8text().c_str());
                    }
                    return true;
                }
            }
        }
        //We donnot need the key 
        event.reject();
        return false;
    }
    bool TextBox::handle_textinput(TextInputEvent &event){
        add_string(event.text);
        editing = false;
        return event.accept();
    }
    bool TextBox::handle_textediting(TextEditingEvent &event){
        // editing = true;
        // BTK_LOGINFO("Editing");
        return event.accept();
    }
    bool TextBox::handle_drag(DragEvent &event){
        switch(event.type()){
            case Event::DragBegin:{
                is_dragging = true;
                break;
            }
            case Event::Drag:{
                //TODO
                break;
            }
            case Event::DragEnd:{
                is_dragging = false;
                break;
            }
            default:{}
        }
        return event.accept();
    }
    bool TextBox::handle_mouse(MouseEvent &event){
        return event.accept();
    }
    void TextBox::u8text(std::string &s) const{
        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(s));
    }
    void TextBox::set_text(std::u16string_view txt){
        tb_text = txt;
        //tb_buf = nullptr;
        cur_txt = tb_text.begin();
        redraw();
    }
    void TextBox::set_text(u8string_view _txt){
        tb_text.clear();
        auto txt = _txt.base();
        utf8to16(txt.begin(),txt.end(),back_inserter(tb_text));
        //tb_buf = nullptr;
        cur_txt = tb_text.begin();
        redraw();
    }
    void TextBox::add_string(u8string_view _text){
        if(_text.length() == 0){
            return;
        }
        std::u16string::iterator iter;

        if(cur_txt == --tb_text.begin()){
            iter = tb_text.begin();
        }
        else{
            iter = cur_txt;
            //Not at end,point to the next ch
            if(iter != tb_text.end()){
                ++iter;
            }
        }
        //Check the string is valid
        BTK_ASSERT(_text.is_vaild());
        auto text = _text.base();
        auto end = utf8to16(text.begin(),text.end(),TextBoxInserter{*this,iter});
        cur_txt = --(end.cur);
        #ifndef NDEBUG
        std::string t;
        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
        BTK_LOGINFO("TextBox:text=%s",t.c_str());
        #endif
        //tb_buf = nullptr;

        redraw();
    }
    void TextBox::timeout(){
        show_line = not show_line;
        redraw();
    }
}
