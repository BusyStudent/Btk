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
    static u16string get_font_from_name(Win32::RegKey &regkey,const char16_t *name){
        wchar_t buffer[MAX_PATH + 1];//Return value buffer
        buffer[MAX_PATH] = '\0';

        DWORD length = MAX_PATH;
        LSTATUS ret;
        ret = RegQueryValueExW(
            regkey,
            reinterpret_cast<const wchar_t*>(name),
            nullptr,
            nullptr,
            reinterpret_cast<LPBYTE>(buffer),
            &length
        );
        if(ret == ERROR_SUCCESS){
            //Succeed to find the name
            const char16_t *buf = reinterpret_cast<const char16_t*>(buffer);
            BTK_LOGINFO("Match font => %s",u16string_view(buf).to_utf8().c_str());
            u16string ret("C:/Windows/Fonts/");
            ret.append(buf);
            return ret;
        }
        else{
            //It should not happened
            throwWin32Error(GetLastError());
        }
    }
    static u16string get_file_by_name_impl(const u16string &name){
        using Win32::StrMessageA;
        using Win32::RegKey;

        RegKey regkey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts");
        //Could not open
        if(not regkey.ok()){
            throw Win32Error(GetLastError());
        }
        LSTATUS ret;
        
        wchar_t buffer[MAX_PATH + 1];//Return value buffer
        buffer[MAX_PATH] = '\0';

        DWORD index = 0;
        DWORD buflen = MAX_PATH;

        do{
            ret = RegEnumValueW(
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
                BTK_LOGINFO("Name of %s",u16string_view((const char16_t*)buffer).to_utf8().c_str());
                if(SDL_wcsncasecmp(name.w_str(),buffer,name.length()) == 0){
                    return get_font_from_name(regkey,reinterpret_cast<char16_t*>(buffer));
                }
                else if(SDL_wcsstr(buffer,name.w_str()) != 0){
                    //Like xx & req
                    return get_font_from_name(regkey,reinterpret_cast<char16_t*>(buffer));
                }
                else{
                    //Reset the buffer
                    memset(buffer,'\0',buflen * sizeof(wchar_t));
                    buflen = MAX_PATH;
                }
            }
            else{
                BTK_LOGINFO("RegEnumKeyExA => %s",StrMessageA(GetLastError()).c_str());
            }
            index ++;
        }
        while(ret == ERROR_SUCCESS);
        return {};
    }
    //It need impove if the value is 'MS Gothic & MS UI Gothic & MS PGothic (TrueType)'
    u8string GetFileByName(u8string_view name){
        using Win32::StrMessageA;
        using Win32::RegKey;

        if(name.empty()){
            //GetDefaultFont
            return GetDefaultFont();
        }
        u16string s = get_file_by_name_impl(name.to_utf16());
        if(s.empty()){
            return GetDefaultFont();
        }
        return s.to_utf8();
    }
    //Callback for enum font
    static int CALLBACK font_cb(const LOGFONTA *font,const TEXTMETRICA*,DWORD type,LPARAM data){
        if(type == TRUETYPE_FONTTYPE){
            //Is truetype
            BTK_LOGINFO(font->lfFaceName);
            ((u8string*)data)->assign(font->lfFaceName);
            return 0;
        }
        return 1;
    };
    u8string GetDefaultFont(){
        NONCLIENTMETRICSW info;
        info.cbSize = sizeof(NONCLIENTMETRICSW);
        if(not SystemParametersInfoW(
            SPI_GETNONCLIENTMETRICS,
            sizeof(NONCLIENTMETRICSW),
            &info,
            0
        )){
            throwWin32Error(GetLastError());
        }
        u16string name = reinterpret_cast<char16_t*>(info.lfMenuFont.lfFaceName);
        //Remove ' UI'
        // name.pop_back();
        // name.pop_back();
        // name.pop_back();

        auto ret = get_file_by_name_impl(name);

        if(ret.empty()){
            //Handle default cannot be founded
        }
        return ret.to_utf8();
    }
}
}


#endif