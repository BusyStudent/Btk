#ifdef _WIN32

#include <windows.h>
#include <wingdi.h>
#include <winreg.h>

//for SDL_strncasecmp
#include <SDL2/SDL_stdinc.h>

#include "../../build.hpp"
#include "internal.hpp"

#include <Btk/platform/win32.hpp>
#include <Btk/impl/utils.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <thread>
#include <mutex>
namespace Btk{
namespace FontUtils{
    void Init(){
    
    }
    void Quit(){

    }
    static std::string get_font_from_name(Win32::RegKey &regkey,const char *name){
        char buffer[MAX_PATH];
        DWORD length = MAX_PATH;
        LSTATUS ret;
        ret = RegQueryValueExA(
            regkey,
            name,
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(buffer),
            &length
        );
        if(ret == ERROR_SUCCESS){
            //Succeed to find the name
            BTK_LOGINFO("Match font => %s",buffer);
            std::string ret("C:/Windows/Fonts/");
            ret += buffer;
            return ret;
        }
        else{
            //It should not happened
            throwWin32Error(GetLastError());
        }
    }
    //It need impove if the value is 'MS Gothic & MS UI Gothic & MS PGothic (TrueType)'
    std::string GetFileByName(std::string_view name){
        using Win32::StrMessageA;
        using Win32::RegKey;

        if(name == ""){
            //GetDefaultFont
            return "C:/Windows/Fonts/msyh.ttc";
        }
        
        char buffer[MAX_PATH + 1];//Return value buffer
        buffer[MAX_PATH] = '\0';

        //Copy into buffer
        auto &u8buf = FillInternalU8Buffer(name);
        RegKey regkey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
        //Could not open
        if(not regkey.ok()){
            throw Win32Error(GetLastError());
        }
        LSTATUS ret;

        DWORD index = 0;
        DWORD buflen = MAX_PATH;

        do{
            ret = RegEnumValueA(
                regkey,
                index,
                buffer,
                &buflen,
                nullptr,
                nullptr,
                nullptr,
                nullptr
            );
            if(ret == ERROR_SUCCESS){
                if(SDL_strncasecmp(name.data(),buffer,name.length()) == 0){
                    return get_font_from_name(regkey,buffer);
                }
                else{
                    //Reset the buffer
                    memset(buffer,'\0',buflen * sizeof(char));
                    buflen = MAX_PATH;
                }
            }
            else{
                BTK_LOGINFO("RegEnumKeyExA => %s",StrMessageA(GetLastError()).c_str());
            }
            index ++;
        }
        while(ret == ERROR_SUCCESS);
        //Failed
        return "C:/Windows/Fonts/msyh.ttc";
    }
    //Callback for enum font
    static int CALLBACK font_cb(const LOGFONTA *font,const TEXTMETRICA*,DWORD type,LPARAM data){
        if(type == TRUETYPE_FONTTYPE){
            //Is truetype
            BTK_LOGINFO(font->lfFaceName);
            ((std::string*)data)->assign(font->lfFaceName);
            return 0;
        }
        return 1;
    };
    std::string GetDefaultFont(){
        using Win32::HandleDC;


        HandleDC hdc(nullptr);
        //Get the charset
        int charset = GetTextCharset(hdc);
        
        BTK_LOGINFO("HDC charset %d",charset);

        std::string facename;//Get the facename
        LOGFONTA font_data;
        SDL_zero(font_data);
        font_data.lfCharSet = charset;
        EnumFontFamiliesExA(
            hdc,
            &font_data,
            font_cb,
            (LPARAM)&facename,
            0
        );
        return GetFileByName(facename);
    }
}
}


#endif