#if !defined(_BTK_IMPL_INPUT_HPP_)
#define _BTK_IMPL_INPUT_HPP_
/**
 * @brief Internal Headers for IME
 * 
 */
#include <SDL2/SDL_clipboard.h>
#include <SDL2/SDL_keyboard.h>
#include "../rect.hpp"
namespace Btk{
    //Alias
    inline void StartTextInput(){
        SDL_StartTextInput();
    }
    inline void StopTextInput(){
        SDL_StopTextInput();
    }
    //Why SDL_SetTextInputRect 's arg is not const
    inline void SetTextInputRect(const Rect *r = nullptr){
        if(r == nullptr){
            SDL_SetTextInputRect(nullptr);
        }
        else{
            SDL_Rect rect = *r;
            SDL_SetTextInputRect(&rect);
        }
    }
    inline void SetTextInputRect(const FRect *r = nullptr){
        if(r == nullptr){
            SDL_SetTextInputRect(nullptr);
        }
        else{
            Rect rect = *r;
            SDL_SetTextInputRect(&rect);
        }
    }
    inline void SetTextInputRect(const Rect &r){
        SetTextInputRect(&r);
    }
    inline void SetClipboardText(const u8string &us){
        SDL_SetClipboardText(us.c_str());
    }
    inline auto GetClipboardText() -> u8string{
        char *s = SDL_GetClipboardText();
        if(s == nullptr){
            return {};
        }
        u8string us(s);
        SDL_free(s);
        return us;
    }
    inline bool HasClipboardText(){
        return SDL_HasClipboardText();
    }
}

#endif // _BTK_IMPL_INPUT_HPP_
