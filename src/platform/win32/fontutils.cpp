#include <windows.h>
#include <wingdi.h>

#include "../../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <thread>
#include <mutex>
namespace Btk{
namespace FontUtils{
    using utf8::unchecked::utf16to8;
    /**
     * @brief The function pointer GetFontResourceInfoW
     * 
     */
    static BOOL(*dgi_getfont)(LPCTSTR,LPDWORD,LPVOID,DWORD);
    void Init(){
        HMODULE handle = GetModuleHandleA("gdi32.dll");
        BTK_ASSERT(handle != nullptr);
        //Get the function ptr
        dgi_getfont = (decltype(dgi_getfont)) GetProcAddress(handle,"GetFontResourceInfoW");
        BTK_ASSERT(dgi_getfont != nullptr);
    };
    void Quit(){

    };
    std::string GetFileByName(std::string_view name){
        if(dgi_getfont == nullptr){
            //Was not init
            Init();
        }
        //SDL_TriggerBreakpoint();
        return "C:/Windows/Fonts/msyh.ttc";
    };
};
};