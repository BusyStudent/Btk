#include "../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/textbox.hpp>
#include <Btk/render.hpp>
#include <Btk/event.hpp>

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
        cur_text = "Hello";
    }
    LineEdit::~LineEdit() = default;
    void LineEdit::draw(Renderer &p){
        auto txt_limit = map_to_root(text_limit_area);
        auto txt_center = map_to_root(text_area);
        auto txt_pos = map_to_root(text_pos);

        p.save();
        //Boarder and background
        p.draw_box(rect,theme().active.background);
        p.draw_rect(rect,theme().active.border);

        //Draw txt
        // p.intersest_scissor(txt_limit);

        // p.use_font(font());
        p.use_font(font());
        p.text_size(18);
        p.draw_text(txt_pos.x,txt_pos.y,cur_text,{0,0,0});
        // p.draw_line({0,0},txt_pos,{0,0,0});

        p.restore();

    }
    auto LineEdit::get_pos_from(const Point &p) -> size_t{
        //Use font
        renderer()->use_font(font());
        FSize size;

        map_to_self(p);
        auto view = cur_text.view();
        size_t len = view.length();

        for(size_t cur = 0;cur < len;++cur){
            size = renderer()->text_size(view.substr(0,cur));

            return cur;
        }
        //Unknown
        return size_t(-1);
    }
    void LineEdit::set_rect(const Rect &r){
        Widget::set_rect(r);
        //Config local
        text_limit_area = map_to_self(rectangle<float>());
        text_pos = {text_limit_area.x + 5,h<float>() / 2};
    }
}