#include "../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/scope.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/textbox.hpp>
#include <Btk/window.hpp>
#include <Btk/cursor.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    using utf8::unchecked::utf8to16;
    using utf8::unchecked::utf16to8;
    //A inserter
    struct TextBoxInserter{
        TextBoxInserter() = default;
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
        switch(event.type()){
            //Accept the keyboard
            case Event::KeyBoard:{
                if(not has_focus){
                    return false;
                }
                event.accept();
                return do_keyboard(event_cast<KeyEvent&>(event));
            }
            case Event::TextInput:{
                if(not has_focus){
                    return false;
                }
                event.accept();
                auto &tevent = event_cast<TextInputEvent&>(event);
                add_string(tevent.text);
                return true;
            }
            case Event::DragBegin:{
                event.accept();
                is_dragging = true;
                return true;
            }
            case Event::Drag:{
                event.accept();
                return true;
            }
            case Event::DragEnd:{
                event.accept();
                is_dragging = false;
                return true;
            }
            case Event::Click:{
                event.accept();
                return true;
            }
            case Event::SetRect:{
                event.accept();
                rect = event_cast<SetRectEvent&>(event).rect;
                return true;
            }
            case Event::TakeFocus:{
                //Enable Text input
                event.accept();
                has_focus = true;
                show_line = true;
                timer.start();
                SDL_SetTextInputRect(&rect);
                SDL_StartTextInput();
                redraw();
                return true;
            }
            case Event::LostFocus:{
                //Stop Text input
                event.accept();
                has_focus = false;
                timer.stop();
                SDL_StopTextInput();
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
    TextBox::TextBox(Container &w){
        parent = &w;
        //Set theme
        theme = *(window()->theme);
        tb_font = window()->default_font;

        //Get rendered text's h
        ft_h = tb_font.height();
        
        BTK_LOGINFO("TextBox %p's ft_h=%d",this,ft_h);
        //Set current position
        cur_txt = tb_text.begin();

        attr.focus = FocusPolicy::Click;

        //Set timer
        timer.set_interval(500);
        timer.set_callback(&TextBox::timeout,this);
    }
    TextBox::~TextBox(){
        timer.stop();
    }

    void TextBox::draw(Renderer &render){
        render.box(rect,{255,255,255,255});
        if(not tb_text.empty()){
            if(tb_buf.empty()){
                tb_buf = tb_font.render_blended(tb_text,theme.text_color);
            }
            texture = render.create_from(tb_buf);
            
            //Render text
            auto cliprect = render.get_cliprect();
            render.set_cliprect(rect);

            Rect txt_rect = rect;
            txt_rect.x += tb_boarder;
            txt_rect.w = tb_buf->w;
            txt_rect.h = tb_buf->h;
            txt_rect.y = CalculateYByAlign(rect,tb_buf->h,Align::Center);
            render.copy(texture,nullptr,&txt_rect);
            
            if(has_focus and show_line){
                //draw a line for editing
                //calc the w
                int line_x;

                if(cur_txt == --tb_text.begin()){
                    //Just point at the rect.x
                    line_x = txt_rect.x;
                }
                else{
                    line_x = txt_rect.x + tb_font.size(std::u16string(tb_text.begin(),cur_txt + 1)).w;
                }
                
                render.line(line_x,txt_rect.y,line_x,txt_rect.y + tb_buf->h,theme.text_color);
            }

            render.set_cliprect(cliprect);
        }
        else if(has_focus and show_line){
            //render the line
            int line_y = CalculateYByAlign(rect,ft_h,Align::Center);
            render.line(rect.x + tb_boarder,
                        line_y,
                        rect.x + tb_boarder,
                        line_y + ft_h,
                        theme.text_color
                        );
        }

        if(has_focus){
            render.rounded_rect(rect,1,theme.high_light);
        }
        else{
            render.rounded_rect(rect,1,theme.border_color);
        }
    }
    bool TextBox::do_keyboard(KeyEvent &event){
        if(event.state == KeyEvent::Pressed){
            switch(event.keycode){
                case SDLK_BACKSPACE:{
                    if(not tb_text.empty() and cur_txt != --tb_text.begin()){
                        
                        cur_txt = tb_text.erase(cur_txt);
                        --cur_txt;

                        #ifndef NDEBUG
                        std::string t;
                        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
                        BTK_LOGINFO("TextBox:text=%s",t.c_str());
                        #endif
                        tb_buf = nullptr;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case SDLK_LEFT:{
                    if(cur_txt != --tb_text.begin()){
                        --cur_txt;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case SDLK_RIGHT:{
                    if(cur_txt != --tb_text.end()){
                        ++cur_txt;
                        show_line = true;
                        redraw();
                    }
                    return true;
                }
                case SDLK_v:{
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
                case SDLK_c:{
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
    void TextBox::u8text(std::string &s) const{
        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(s));
    }
    void TextBox::set_text(std::u16string_view txt){
        tb_text = txt;
        tb_buf = nullptr;
        cur_txt = tb_text.begin();
        redraw();
    }
    void TextBox::set_text(std::string_view txt){
        tb_text.clear();
        utf8to16(txt.begin(),txt.end(),back_inserter(tb_text));
        tb_buf = nullptr;
        cur_txt = tb_text.begin();
        redraw();
    }
    void TextBox::add_string(std::string_view text){
        if(text.length() == 0){
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
        BTK_ASSERT(utf8::is_valid(text.begin(),text.end()));

        auto end = utf8to16(text.begin(),text.end(),TextBoxInserter{*this,iter});
        cur_txt = --(end.cur);
        #ifndef NDEBUG
        std::string t;
        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
        BTK_LOGINFO("TextBox:text=%s",t.c_str());
        #endif
        tb_buf = nullptr;

        redraw();
    }
    void TextBox::timeout(){
        show_line = not show_line;
        redraw();
    }
}