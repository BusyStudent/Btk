#include "build.hpp"

#include <SDL2/SDL_mouse.h>
#include <Btk/exception.hpp>
#include <Btk/cursor.hpp>
#include <Btk/pixels.hpp>
#include <Btk/Btk.hpp>
#include <stack>
namespace Btk{
    inline SDL_SystemCursor TranslateCursor(SystemCursor cursor){
        switch(cursor){
            case SystemCursor::Arrow:
                return SDL_SYSTEM_CURSOR_ARROW;
            case SystemCursor::Crosshair:
                return SDL_SYSTEM_CURSOR_CROSSHAIR;
            case SystemCursor::Wait:
                return SDL_SYSTEM_CURSOR_WAIT;
            case SystemCursor::Ibeam:
                return SDL_SYSTEM_CURSOR_IBEAM;
            case SystemCursor::Hand:
                return SDL_SYSTEM_CURSOR_HAND;
            default:
                return SDL_SYSTEM_CURSOR_NO;
        }
    }
    struct BTKHIDDEN Cursor::Impl{
        SDL_Cursor *cursor;
        int  refcount;
        bool reference = false;//< Should we free it

        void unref() noexcept{
            if(refcount == -1){
                return;
            }

            --refcount;
            if(refcount == 0){
                if(not reference){
                    SDL_FreeCursor(cursor);
                }
                delete this;
            }
        }
        void ref() noexcept{
            if(refcount == -1){
                return;
            }

            ++refcount;
        }
    };
    constexpr int DEFAULT_CURSOR = SDL_NUM_SYSTEM_CURSORS;

    static Cursor::Impl sys_cursors[SDL_NUM_SYSTEM_CURSORS + 1] = {};
    static Constructable<std::stack<Cursor::Impl*>> cursors_stack;
    
    static void cursor_cleanup(){
        cursors_stack.destroy();
        for(int i = 0;i < SDL_NUM_SYSTEM_CURSORS;i++){
            SDL_FreeCursor(sys_cursors[i].cursor);
            sys_cursors[i].cursor = nullptr;
        }
    }

    static void cursor_init(){
        if(sys_cursors[0].cursor == nullptr){
            //Init it
            for(int i = 0;i < SDL_NUM_SYSTEM_CURSORS;i++){
                sys_cursors[i].cursor = SDL_CreateSystemCursor(
                    SDL_SystemCursor(i)
                );
                sys_cursors[i].refcount = -1;
            }
            //set default cursor
            sys_cursors[DEFAULT_CURSOR].cursor = SDL_GetDefaultCursor();
            sys_cursors[DEFAULT_CURSOR].refcount = -1;
            cursors_stack.construct();
            AtExit(cursor_cleanup);
        }
    }
    static void cursor_set_sys(Cursor::Impl *cur){
        if(cur == nullptr){
            throwRuntimeError("empty cursor");
        }

        cursors_stack->push(cur);
        cur->ref();

        SDL_SetCursor(cur->cursor);
    }
    static auto cursor_get_current() -> Cursor::Impl*{
        if(cursors_stack->empty()){
            return &sys_cursors[DEFAULT_CURSOR];
        }
        SDL_Cursor *cur = SDL_GetCursor();
        //Is same as cursor in stack
        if(cur == cursors_stack->top()->cursor){
            auto ptr = cursors_stack->top();
            ptr->ref();
            return ptr;
        }
        //Take a ref in it
        return new Cursor::Impl{
            cur,
            -1,
            true
        };
    }
    void SetCursor(SystemCursor cur){
        cursor_init();
        cursor_set_sys(&sys_cursors[TranslateCursor(cur)]);
    }
    void ResetCursor() noexcept{
        cursor_init();
        SDL_Cursor *cur;

        if(not cursors_stack->empty()){
            //do pop
            cursors_stack->top()->unref();
            cursors_stack->pop();
        }

        if(cursors_stack->empty()){
            //Use default
            cur = SDL_GetDefaultCursor();
        }
        else{
            //Use prev
            cur = cursors_stack->top()->cursor;
        }

        SDL_SetCursor(cur);
    }

    Cursor::Cursor(SystemCursor c){
        cursor_init();
        cursor = &sys_cursors[TranslateCursor(c)];
    }
    Cursor::Cursor(PixBufRef pixbuf,int hot_x,int hot_y){
        cursor_init();
        cursor = new Impl{
            SDL_CreateColorCursor(pixbuf.get(),hot_x,hot_y),
            -1
        };
        if(cursor->cursor == nullptr){
            delete cursor;
            throwSDLError();
        }
        
    }
    Cursor Cursor::Default(){
        cursor_init();
        return &sys_cursors[DEFAULT_CURSOR];
    }
    Cursor Cursor::Current(){
        cursor_init();
        return cursor_get_current();
    }

    void Cursor::_Ref(void *p) noexcept{
        if(p == nullptr){
            return;
        }
        static_cast<Impl*>(p)->ref();
    }
    void Cursor::_Unref(void *p) noexcept{
        if(p == nullptr){
            return;
        }
        static_cast<Impl*>(p)->unref();
    }


    void Cursor::_Set(void *p) {
        cursor_set_sys(static_cast<Impl*>(p));
    }

}