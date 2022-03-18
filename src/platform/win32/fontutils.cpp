#ifdef _WIN32

#include <windows.h>
#include <wingdi.h>
#include <winreg.h>

//for SDL_strncasecmp
#include <SDL2/SDL_stdinc.h>

#include "../../build.hpp"
#include "internal.hpp"

#include <Btk/platform/win32.hpp>
#include <Btk/detail/utils.hpp>
#include <Btk/utils/mem.hpp>
#include <Btk/font.hpp>
#include <Btk/Btk.hpp>
#include <thread>
#include <mutex>

namespace Btk::FontUtils{
    void Init(){
    
    }
    void Quit(){

    }
    static u16string get_font_from_name(Win32::RegKey &regkey,const wchar_t *name){
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
            if(ret != ERROR_SUCCESS){
                //No more value
                break;
            }
            index ++;
            // BTK_LOGINFO("Name of %s",u16string_view((const char16_t*)buffer).to_utf8().c_str());
            if(SDL_wcsncasecmp(name.w_str(),buffer,name.length()) == 0){
                return get_font_from_name(regkey,buffer);
            }
            else if(SDL_wcsstr(buffer,name.w_str()) != 0){
                //Like xx & req
                return get_font_from_name(regkey,buffer);
            }
            //Reset the buffer
            memset(buffer,'\0',buflen * sizeof(wchar_t));
            buflen = MAX_PATH;
        }
        while(true);
        return {};
    }
    //Callback for enum font
    template<typename Param>
    static BOOL CALLBACK gdi_callback(const LOGFONTW *font,const TEXTMETRICW*w,DWORD type,LPARAM param){
        Param *p = reinterpret_cast<Param*>(param);
        return (*p)(font,w,type);
    };
    template<class Callable,class ...Args>
    static void gdi_enum_font(Callable &&callable,Args &&...args){

        auto callback_forward = [&](const LOGFONTW *font,const TEXTMETRICW*w,DWORD type) -> BOOL{
            //Does the callback has return value?
            constexpr bool has_return_type = std::is_same_v<
                std::invoke_result_t<Callable,const LOGFONTW&,const TEXTMETRICW&,DWORD,Args...>,
                bool
            >;

            if constexpr(has_return_type){
                return callable(*font,*w,type,std::forward<Args>(args)...);
            }
            else{
                callable(*font,*w,type,std::forward<Args>(args)...);
                return TRUE;
            }
        };
        auto gdi_font_cb = gdi_callback<decltype(callback_forward)>;

        HDC hdc = GetDC(nullptr);
        if(hdc == nullptr){
            throwWin32Error(GetLastError());
        }
        //EnumFontFamiliesExW
        if(!EnumFontFamiliesExW(hdc,nullptr,gdi_font_cb,reinterpret_cast<LPARAM>(&callback_forward),0)){
            throwWin32Error(GetLastError());
        }
        ReleaseDC(nullptr,hdc);
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
    //TODO
    FontInfo FindFont(u8string_view name){
        FontInfo info;
        info.filename = GetFileByName(name);
        info.index = 0;
        return info;
    }
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
#endif