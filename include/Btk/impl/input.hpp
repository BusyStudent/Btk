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
    void StartTextInput(){
        SDL_StartTextInput();
    }
    void StopTextInput(){
        SDL_StopTextInput();
    }
    //Why SDL_SetTextInputRect 's arg is not const
    void SetTextInputRect(const Rect *r = nullptr){
        if(r == nullptr){
            SDL_SetTextInputRect(nullptr);
        }
        SDL_Rect rect = *r;
        SDL_SetTextInputRect(&rect);
    }
    void SetTextInputRect(const Rect &r){
        SetTextInputRect(&r);
    }
}

#endif // _BTK_IMPL_INPUT_HPP_
