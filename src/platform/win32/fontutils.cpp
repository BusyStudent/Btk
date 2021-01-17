#ifdef _WIN32

#include <windows.h>
#include <wingdi.h>

#include "../../build.hpp"

#include <Btk/thirdparty/utf8.h>
#include <Btk/utils/mem.hpp>
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
    static BOOL (*dgi_getfont)(char16_t *,LPDWORD,LPVOID,DWORD);
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
        if(dgi_getfont == nullptr){
            //fail to get the function
            goto err;
        }
        //SDL_TriggerBreakpoint();
        std::u16string wname;
        Utf8To16(wname,name);
        BOOL  ret;
        DWORD bufsize;

        ret = dgi_getfont(wname.data(),&bufsize,nullptr,1);
        std::u16string str;
        str.resize(bufsize);
        ret = dgi_getfont(wname.data(),&bufsize,str.data(),1);

        BTK_LOGINFO("GetFontResourceInfoW:%d",int(ret));
        if(not ret){
            //Failed to get the font name
            err:
            return "C:/Windows/Fonts/msyh.ttc";
        }
        std::string u8;
        Utf16To8(u8,str);

        BTK_LOGINFO("Match font => %s",u8.c_str());

        return u8;
    };
};
};

#endif