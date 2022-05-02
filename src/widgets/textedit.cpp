#include "../build.hpp"

#include <Btk/detail/input.hpp>
#include <Btk/textbox.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>

#include "../libs/utf8.h"

#if 0

//Keycode
#define STB_TEXTEDIT_K_SHIFT Uint32(1 << 0)
#define STB_TEXTEDIT_K_RIGHT  Uint32(1 << 1)
#define STB_TEXTEDIT_K_LEFT  Uint32(1 << 2)
#define STB_TEXTEDIT_K_UNDO  Uint32(1 << 3)
#define STB_TEXTEDIT_K_REDO  Uint32(1 << 4)
#define STB_TEXTEDIT_K_DOWN  Uint32(1 << 5)
#define STB_TEXTEDIT_K_PGDOWN  Uint32(1 << 6)
#define STB_TEXTEDIT_K_PGUP  Uint32(1 << 7)
#define STB_TEXTEDIT_K_UP  Uint32(1 << 8)
#define STB_TEXTEDIT_K_DELETE  Uint32(1 << 9)
#define STB_TEXTEDIT_K_BACKSPACE  Uint32(1 << 10)
#define STB_TEXTEDIT_K_TEXTSTART  Uint32(1 << 11)
#define STB_TEXTEDIT_K_TEXTEND  Uint32(1 << 12)
#define STB_TEXTEDIT_K_LINESTART  Uint32(1 << 13)
#define STB_TEXTEDIT_K_LINEEND  Uint32(1 << 14)


//Default not invalid to insert
#define STB_TEXTEDIT_KEYTOTEXT(K) -1

#define STB_TEXTEDIT_GETWIDTH_NEWLINE -1
#define STB_TEXTEDIT_CHARTYPE   char32_t
#define STB_TEXTEDIT_KEYTYPE    Uint32
#define STB_TEXTEDIT_STRING     Btk::AbstractEditor::_Internal
#define STB_TEXTEDIT_NEWLINE    U'\n'
//Forward decl
static size_t   STB_TEXTEDIT_STRINGLEN(STB_TEXTEDIT_STRING *s);
static char32_t STB_TEXTEDIT_GETCHAR(STB_TEXTEDIT_STRING *,int i);
static bool     STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING *,int index,const char32_t *text,int n);
static bool     STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING *,int index,int n);
static float    STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING *,int n,int i);
static void     STB_TEXTEDIT_LAYOUTROW(void *result,STB_TEXTEDIT_STRING *,int index);

#define STB_TEXTEDIT_IMPLEMENTATION
#include "../libs/stb_textedit.h"
#include "../libs/nanovg.h"

//LineEdit Impl
namespace Btk{
    struct BTKHIDDEN AbstractEditor::_Internal{
        u32string text;
        u8string cvt_buf;
        STB_TexteditState state;
        AbstractEditor *editor;

        const char32_t *cur_insert;
        int  fontsize;
        int  cur_insert_length;
        int  cur_delete_length;

        inline
        bool  on_insert_text(int position,const char32_t *text,int n);
        inline
        bool  on_delete_text(int position,int n);
        inline
        float get_width(int n,int i);
        inline
        void  layout_row(StbTexteditRow *result,int index);
        inline 
        void  check_range(int position){
            BTK_ASSERT(position < text.length());
        }
    };
    //Impl
    inline
    bool  AbstractEditor::_Internal::on_insert_text(int position,const char32_t *text,int n){
        cur_insert = text;
        cur_insert_length = n;
        if(editor->editor_on_insert(position)){
            //Exec insert
            this->text.insert(position,text,n);
            return true;
        }
        sizeof(AbstractEditor::_Internal);
        return false;
    }
    inline
    bool  AbstractEditor::_Internal::on_delete_text(int position,int n){
        cur_delete_length = n;
        if(editor->editor_on_delete(position)){
            //Exec insert
            text.erase(position,n);
            return true;
        }
        return false;
    }
    inline
    float AbstractEditor::_Internal::get_width(int n,int i){
        auto renderer = editor->renderer();
        auto context = renderer->context();

        auto size = Utf32CharSize(text[n + i]);
        auto cur = &text[n + i];
        if (size == 1 and *cur == '\n'){
            return STB_TEXTEDIT_GETWIDTH_NEWLINE;
        }

        //Convert the utf32 to 8
        std::array<char,4> buf;
        //Use Default font
        Utf32ToUtf8(*cur,buf.data(),buf.size());
        nvgFontFaceId(context,0);
        nvgFontSize(context,fontsize);
        
        return nvgTextBounds(context,0,0,&buf[0],&buf[size],nullptr);
    }
    inline
    void  AbstractEditor::_Internal::layout_row(StbTexteditRow *result,int index){
        auto renderer = editor->renderer();
        auto context = renderer->context();
        //Convert
        cvt_buf.clear();
        utf8::unchecked::utf32to8(&text[index],&text[text.length()],std::back_inserter(cvt_buf.base()));

        auto start = cvt_buf->begin();
        NVGtextRow row;

        if (nvgTextBreakLines(context, &*start,nullptr,editor->w(), &row, 1) == false) {
            result->x0 = result->x1 = 0;
            result->ymin = result->ymax = 0;
            result->baseline_y_delta = 1.25;
            result->num_chars = 0;
            return;
        }

        float bounds[4]; // xmin, ymin, xmax, ymax
        nvgTextBounds(context,0,0,row.start,row.end,bounds);

        result->x0 = bounds[0];
        result->x1 = bounds[2];
        result->ymin = bounds[1];
        result->ymax = bounds[3];
        result->baseline_y_delta = 1.25;
        result->num_chars = text.length() - index;

    }

    AbstractEditor::AbstractEditor(){
        context = new _Internal;
        context->editor = this;
    }
    AbstractEditor::~AbstractEditor(){
        delete context;
    }
    void AbstractEditor::editor_init(bool single_lines){
        stb_textedit_initialize_state(&(context->state),single_lines);
    }
    bool AbstractEditor::handle_keyboard(KeyEvent &event){
        Uint32 key = 0;
        //Tr code
        switch(event.keycode){
            case Keycode::Right:
                key = STB_TEXTEDIT_K_RIGHT;
                break;
            case Keycode::Left:
                key = STB_TEXTEDIT_K_LEFT;
                break;
            case Keycode::Undo:
                key = STB_TEXTEDIT_K_UNDO;
                break;
            case Keycode::Redo:
                key = STB_TEXTEDIT_K_REDO;
                break;
            case Keycode::Down:
                key = STB_TEXTEDIT_K_DOWN;
                break;
            case Keycode::Pagedown:
                key = STB_TEXTEDIT_K_PGDOWN;
                break;
            case Keycode::Pageup:
                key = STB_TEXTEDIT_K_PGUP;
                break;
            case Keycode::Up:
                key = STB_TEXTEDIT_K_UP;
                break;
            case Keycode::Delete:
                key = STB_TEXTEDIT_K_DELETE;
                break;
            case Keycode::Backspace:
                key = STB_TEXTEDIT_K_BACKSPACE;
                break;
            case Keycode::Home:
                if(event.has_kmod(Keymode::Ctrl)){
                    key = STB_TEXTEDIT_K_TEXTSTART;
                }
                else{
                    key = STB_TEXTEDIT_K_LINESTART;
                }
                break;
            case Keycode::End:
                if(event.has_kmod(Keymode::Ctrl)){
                    key = STB_TEXTEDIT_K_TEXTEND;
                }
                else{
                    key = STB_TEXTEDIT_K_LINEEND;
                }
                break;
            default:
                return false;
        }
        if(event.has_kmod(Keymode::Shift)){
            key |= STB_TEXTEDIT_K_SHIFT;
        }
        //Send it to stb
        stb_textedit_key(context,&(context->state),key);
        return event.accept();
    }
}

//STB Forwarder

static size_t STB_TEXTEDIT_STRINGLEN(STB_TEXTEDIT_STRING *s){
    return s->text.length();
}
static char32_t STB_TEXTEDIT_GETCHAR(STB_TEXTEDIT_STRING *s,int i){
    return s->text[i];
}
static bool STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING *s,int index,const char32_t *text,int n){
    return s->on_insert_text(index,text,n);
}
static bool STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING *s,int index,int n){
    return s->on_delete_text(index,n);
}
static float STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING *s,int n,int i){
    return s->get_width(n,i);
}
static void  STB_TEXTEDIT_LAYOUTROW(void *result,STB_TEXTEDIT_STRING *s,int index){
    return s->layout_row(static_cast<StbTexteditRow*>(result),index);
}

#endif

namespace Btk{
    LineEdit::LineEdit(){
        attr.focus = FocusPolicy::Mouse;
        // cur_text = "Hello";
    }
    LineEdit::~LineEdit() = default;
    bool LineEdit::handle(Event &event){
        if(Widget::handle(event)){
            return true;
        }
        if(event.type() == Event::TakeFocus){
            SetTextInputRect(rectangle());
            StartTextInput();
            BTK_LOGINFO("LineEdit take focus");

            show_cur = true;
            has_focus = true;
            redraw();
            return event.accept();
        }
        else if(event.type() == Event::LostFocus){
            StopTextInput();
            BTK_LOGINFO("LineEdit lost focus");
            show_cur = false;
            has_focus = false;
            redraw();
            return event.accept();
        }

        return false;
    }
    bool LineEdit::handle_mouse(MouseEvent &event){
        if(event.is_pressed()){
            size_t pos = get_pos_from(event.position());
            BTK_LOGINFO("LineEdit::handle_mouse %d",int(pos));

            cur_pos = pos;
            //Reset empty selection
            clear_sel();

            redraw();
        }
        return event.accept();
    }
    bool LineEdit::handle_keyboard(KeyEvent &event){
        if(event.is_released()){
            return event.accept();
        }
        switch(event.keycode){
            case Keycode::Backspace:{
                if(has_selection()){
                    //Delete selection
                    auto [start,end] = sel_range();

                    cur_text.erase(
                        start,
                        end - start
                    );

                    cur_pos = start;

                    clear_sel();
                    redraw();
                    //Notify
                    _signal_value_changed.defer_emit();
                }
                else if(not cur_text.empty() and cur_pos > 0){
                    //Normal delete
                    cur_text.erase(cur_pos - 1);
                    if(cur_pos != 0){
                        cur_pos --;
                    }
                    redraw();
                    //Notify
                    _signal_value_changed.defer_emit();
                }
                break;
            }
            case Keycode::Right:{
                if(cur_pos < cur_text.length()){
                    cur_pos ++;
                }
                clear_sel();
                redraw();
                break;
            }
            case Keycode::Left:{
                if(cur_pos > 0){
                    cur_pos --;
                }
                clear_sel();
                redraw();
                break;
            }
            case Keycode::V:{
                //Ctrl + V
                if(event.has_kmod(Keymode::Ctrl)){
                    BTK_LOGINFO("Ctrl + V");
                    if(HasClipboardText()){
                        do_paste(GetClipboardText());
                    }
                } 
                break;
            }
            case Keycode::C:{
                //Ctrl + C
                if(event.has_kmod(Keymode::Ctrl)){
                    BTK_LOGINFO("Ctrl + C");
                    if(has_selection()){
                        SetClipboardText(selection());
                    }
                    else{
                        SetClipboardText(cur_text);
                    }
                }
                break;
            }
            case Keycode::X:{
                if(event.has_kmod(Keymode::Ctrl) and has_selection()){
                    BTK_LOGINFO("Ctrl + X");
                    //Set to clipbiard
                    SetClipboardText(selection());
                    //Remove it
                    auto [start,end] = sel_range();
                    cur_text.erase(
                        start,
                        end - start
                    );
                    cur_pos = start;

                    clear_sel();
                    redraw();
                    //Notify
                    _signal_value_changed.defer_emit();
                }
                break;
            }
            case Keycode::Kp_Enter:{
                if(not _signal_enter_pressed.empty()){
                    _signal_enter_pressed.defer_emit();
                    break;
                }
                [[fallthrough]];
            }
            default:{
                return event.reject();
            }
        }
        return event.accept();
    }
    bool LineEdit::handle_textinput(TextInputEvent &event){
        if(event.text.size() == 0){
            return event.accept();
        }
        do_paste(event.text);
        return event.accept();
    }
    bool LineEdit::handle_drag(DragEvent &event){
        switch(event.type()){
            case Event::DragBegin:
                //Set to current pos
                has_sel = true;
                sel_beg = cur_pos;
                sel_end = cur_pos;
                break;
            case Event::DragEnd:
                if(sel_beg == sel_end){
                    //No text is selected
                    clear_sel();
                }
                break;
            case Event::Drag:
                sel_end = get_pos_from(event.position());
                break;
        }
        redraw();
        return event.accept();
    }
    void LineEdit::do_paste(u8string_view txt){
        BTK_LOGINFO("LineEdit::paste %s",txt.data());

        if(has_selection()){
            auto [start,end] = sel_range();

            cur_text.replace(
                start,
                end - start,
                txt
            );

            //Make the cur to the pasted end
            cur_pos = start + txt.length();

            clear_sel();
        }
        else{
            cur_text.insert(cur_pos,txt);
            cur_pos += txt.length();
        }

        redraw();
        _signal_value_changed.defer_emit();
    }
    void LineEdit::clear_sel(){
        has_sel = false;
        sel_beg = 0;
        sel_end = 0;
    }
    void LineEdit::set_text(u8string_view txt){
        cur_text = txt;
        cur_pos = 0;
        redraw();
        _signal_value_changed.defer_emit();
    }
    void LineEdit::draw(Renderer &p,Uint32){

        p.save();
        //Boarder and background
        p.draw_box(rect,theme().active.background);
        if(has_focus){
            p.draw_rect(rect,theme().active.highlight);
        }
        else{
            p.draw_rect(rect,theme().active.border);
        }

        //Draw txt
        p.intersest_scissor(rectangle());

        // p.use_font(font());
        p.use_font(font());
        p.text_align(align);
        if(not cur_text.empty()){
            p.draw_text(text_pos.x,text_pos.y,cur_text,theme().active.text);
        }
        if(show_cur){
            //Draw cursor
            if(cur_pos == 0 and cur_text.empty()){
                //TODO
                float h = p.font_height();
                float y = text_pos.y - h / 2;
                p.draw_line(
                    text_pos.x,y,
                    text_pos.x,y + h,
                    theme().active.text
                );
            }
            else{
                auto bounds = p.text_bounds(text_pos.x,text_pos.y,cur_text);
                auto vec = p.glyph_positions(text_pos.x,text_pos.y,cur_text);
                auto get = [&](size_t idx){
                    if(idx == vec.size()){
                        return vec.back().maxx;
                    }
                    return vec.at(idx).minx;
                };
                float x = get(cur_pos);
                if(has_selection()){
                    //Draw selection
                    auto [start,end] = sel_range();

                    float sel_x = get(start);
                    float sel_w = get(end) - sel_x;
                    float sel_h = bounds.cast<FRect>().h;
                    float sel_y = bounds.cast<FRect>().y;

                    //Draw 
                    p.draw_box(sel_x,sel_y,sel_w,sel_h,theme().active.highlight);
                    p.intersest_scissor(sel_x,sel_y,sel_w,sel_h);
                    //Draw HeightText back
                    p.draw_text(text_pos.x,text_pos.y,cur_text,theme().active.highlight_text);
                    p.reset_scissor();
                }
                
                //Draw cursor
                p.draw_line(
                    x,bounds.miny,
                    x,bounds.maxy,
                    theme().active.text
                );

            }
        }

        p.restore();

    }
    auto LineEdit::selection() const -> u8string_view{
        auto [start,end] = sel_range();
        return cur_text.view().substr(start,end - start);
    }
    auto LineEdit::get_pos_from(const Point &p) -> size_t{
        //Use font
        if(cur_text.empty()){
            return 0;
        }

        renderer()->use_font(font());
        renderer()->text_align(align);
        FSize size;

        auto view = cur_text.view();
        auto vec = renderer()->glyph_positions(text_pos.x,text_pos.y,view);

        size_t cur = 0;
        for(auto &i:vec){
            if(p.x >= i.minx and p.x <= i.maxx or (&i == &vec.back() and p.x >= i.minx)){
                //Got
                float w = i.maxx - i.minx;
                if(p.x - i.x < w / 2){
                    return cur;
                }
                else{
                    return cur + 1;
                }                    
            }
            cur++;
        }
        //Unknown => First
        return size_t(0);
    }
    void LineEdit::set_rect(const Rect &r){
        Widget::set_rect(r);
        //Config local
        FRect area = limit_margin.apply(r);
        text_pos = {
            area.x,
            area.y + area.h / 2
        };
        //Plus 2 to make it look better
        text_pos.y += 2;
    }
}