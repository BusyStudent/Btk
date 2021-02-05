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
    static BOOL (*dgi_getfont)(const char16_t *,LPDWORD,LPVOID,DWORD);
    /**
     * @brief Internal function wrapper for GetFontResourceInfoW
     * 
     * @param wname The font face name
     * @return The font filename(empty on failure) 
     */
    static std::u16string sys_getfont(const std::u16string &wname){
        if(dgi_getfont == nullptr){
            //Was not init
            Init();
        }
        if(dgi_getfont == nullptr){
            //error
            return std::u16string();
        }
        BOOL  ret;
        DWORD bufsize = 0;

        ret = dgi_getfont(wname.data(),&bufsize,nullptr,1);
        std::u16string str;
        str.resize(bufsize);
        ret = dgi_getfont(wname.data(),&bufsize,str.data(),1);
        BTK_LOGINFO("GetFontResourceInfoW:%d",int(ret));
        if(ret){
            //failed
            return std::u16string();
        }
        else{
            return str;
        }
    }

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
        std::string u8 = Utf16To8(sys_getfont(Utf8To16(name)));
        BTK_LOGINFO("Match font => %s",u8.c_str());
        if(u8.empty()){
            return "C:/Windows/Fonts/msyh.ttc";
        }
        return u8;
    };
};
};

#endif