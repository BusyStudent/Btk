#include <SDL2/SDL_mouse.h>
#include "build.hpp"
#include <Btk/exception.hpp>
#include <Btk/cursor.hpp>
#include <Btk/pixels.hpp>
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
    Cursor::~Cursor(){
        if(own){
            SDL_FreeCursor(cursor);
        }
    }
    Cursor::Cursor(SystemCursor sys){
        own = true;
        cursor = SDL_CreateSystemCursor(TranslateCursor(sys));
        if(cursor == nullptr){
            throwSDLError();
        }
    }
    Cursor::Cursor(const PixBuf &pixbuf,int x,int y){
        own = true;
        cursor = SDL_CreateColorCursor(pixbuf.get(),x,y);
        if(cursor == nullptr){
            throwSDLError();
        }
    }

    Cursor Cursor::Default(){
        SDL_Cursor *cursor = SDL_GetDefaultCursor();
        if(cursor == nullptr){
            throwSDLError();
        }
        return {
            cursor,
            false
        };
    }
    Cursor Cursor::Current(){
        SDL_Cursor *cursor = SDL_GetCursor();
        if(cursor == nullptr){
            throwSDLError();
        }
        return {
            cursor,
            false
        };
    }
    void Cursor::reset(){
        SDL_SetCursor(SDL_GetDefaultCursor());
    }
    void Cursor::set() const{
        SDL_SetCursor(cursor);
    }
}