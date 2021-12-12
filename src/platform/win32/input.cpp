#include "../../build.hpp"

#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_syswm.h>

#include <Btk/impl/window.hpp>
#include <Btk/impl/core.hpp>
#include <Btk/window.hpp>
#include <Btk/Btk.hpp>
#include <windows.h>
#include <imm.h>

#include "internal.hpp"

namespace Btk::WinIme{
    void StartTextInput(){

    }
    void StopTextInput(){

    }
    void SetTextInputRect(const Rect &r){
        
    }
}
namespace Btk::Win32{
    // static WindowImpl *GetWindowFromHWND(void *hwnd){
    //     if(hwnd == nullptr){
    //         return nullptr;
    //     }
    //     std::lock_guard locker(Instance().map_mtx);
    //     for(auto i:Instance().wins_map){
    //         auto ctxt = GetContext(i.second->sdl_window());
    //         if(ctxt.window == hwnd){
    //             return i.second;
    //         }
    //     }
    //     return nullptr;
    // }

    // void SDLCALL MessageHook(void *, void *hwnd, UINT message, UINT64 wParam, INT64 lParam){

    // }
}