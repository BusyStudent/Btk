#include "../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/impl/window.hpp>
#include <Btk/impl/render.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/textbox.hpp>
#include <Btk/window.hpp>
#include <Btk/event.hpp>

#include <algorithm>

namespace Btk{
    using utf8::unchecked::utf8to16;
    using utf8::unchecked::utf16to8;
    using std::back_inserter;
    bool TextBox::handle(Event &event){
        switch(event.type()){
            //Accept the keyboard
            case Event::KeyBoard:{
                if(not has_focus){
                    return false;
                }
                event.accept();
                do_keyboard(event_cast<KeyEvent&>(event));
                return true;
            }
            case Event::TextInput:{
                if(not has_focus){
                    return false;
                }
                event.accept();
                auto &tevent = event_cast<TextInputEvent&>(event);
                utf8to16(tevent.text.begin(),tevent.text.end(),back_inserter(tb_text));
                
                #ifndef NDEBUG
                std::string t;
                utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
                BTK_LOGINFO("TextBox:text=%s",t.c_str());
                #endif
                tb_buf = nullptr;
                texture = nullptr;
                win->draw();
                return true;
            }
            case Event::DragBegin:{
                event.accept();
                return true;
            }
            case Event::Drag:{
                event.accept();
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
                SDL_StartTextInput();
                SDL_SetTextInputRect(&rect);
                win->draw();
                return true;
            }
            case Event::LostFocus:{
                //Stop Text input
                event.accept();
                has_focus = false;
                SDL_StopTextInput();
                win->draw();
                return true;
            }
        }
        return false;
    }
    TextBox::TextBox(Window &w){
        win = &w;
        //Set theme
        theme = *(w.impl()->theme);
        tb_font = w.impl()->default_font;

        //Get rendered text's h
        ft_h = tb_font.size("T").h;
        BTK_LOGINFO("TextBox %p's ft_h=%d",this,ft_h);
    }
    TextBox::~TextBox(){}

    void TextBox::draw(Renderer &render){
        render.box(rect,{255,255,255,255});
        render.rounded_rect(rect,1,theme.border_color);
        if(not tb_text.empty()){
            if(texture.empty()){
                if(tb_buf.empty()){
                    tb_buf = tb_font.render_blended(tb_text,theme.text_color);
                }
                texture = render.create_from(tb_buf);
            }
            //Render text
            auto cliprect = render.get_cliprect();
            render.set_cliprect(rect);

            Rect txt_rect = rect;
            txt_rect.x += tb_boarder;
            txt_rect.w = tb_buf->w;
            txt_rect.h = tb_buf->h;
            txt_rect.y = CalculateYByAlign(rect,tb_buf->h,Align::Center);
            render.copy(texture,nullptr,&txt_rect);
            
            if(has_focus){
                //draw a line for editing
                int line_x = txt_rect.x + tb_buf->w;
                render.line(line_x,txt_rect.y,line_x,txt_rect.y + tb_buf->h,theme.text_color);
            }

            render.set_cliprect(cliprect);
        }
        else if(has_focus){
            //render the line
            int line_y = CalculateYByAlign(rect,ft_h,Align::Center);
            render.line(rect.x + tb_boarder,
                        line_y,
                        rect.x + tb_boarder,
                        line_y + ft_h,
                        theme.text_color
                        );
        }
    }
    void TextBox::do_keyboard(KeyEvent &event){
        if(event.keycode == SDLK_BACKSPACE and event.state == KeyEvent::Pressed){
            //utf8::unchecked::iterator iter = tb_text.end();
            if(not tb_text.empty()){
                tb_text.pop_back();
                #ifndef NDEBUG
                std::string t;
                utf16to8(tb_text.begin(),tb_text.end(),back_inserter(t));
                BTK_LOGINFO("TextBox:text=%s",t.c_str());
                #endif
                tb_buf = nullptr;
                texture = nullptr;
                win->draw();
            }
        }
    }
    void TextBox::u8text(std::string &s) const{
        utf16to8(tb_text.begin(),tb_text.end(),back_inserter(s));
    }
    void TextBox::set_text(std::u16string_view txt){
        tb_text = txt;
        tb_buf = nullptr;
        texture = nullptr;
        win->draw();
    }
    void TextBox::set_text(std::string_view txt){
        tb_text.clear();
        utf8to16(txt.begin(),txt.end(),back_inserter(tb_text));
        tb_buf = nullptr;
        texture = nullptr;
        win->draw();
    }
}