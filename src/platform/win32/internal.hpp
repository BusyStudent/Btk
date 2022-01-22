#if !defined(_BTK_WIN32_INTERNAL_HPP_)
#define _BTK_WIN32_INTERNAL_HPP_
#include <Btk/platform/win32.hpp>
#include <ShlObj.h>
#include <wingdi.h>
#include <winreg.h>

#include <string>

#define WIN32_WSTR (wchar_t*)
#define WIN32_CWSTR (const wchar_t *)
struct SDL_Window;
namespace Btk{
namespace Win32{
    /**
     * @brief RAII for HDC
     * 
     */
    struct HandleDC{
        HandleDC(HWND win){
            window = win;
            dc = GetDC(win);
        }
        HandleDC(const HandleDC &) = delete;
        ~HandleDC(){
            ReleaseDC(window,dc);
        }
        operator HDC() const noexcept{
            return dc;
        }
        HWND window;
        HDC dc;
    };
    /**
     * @brief RAII for reading reg
     * 
     */
    struct RegKey{
        RegKey(HKEY hkey,const char *subkey){
            //Open the key
            if(RegOpenKeyA(hkey,subkey,&key) == ERROR_SUCCESS){
                opened = true;
            }
            else{
                opened = false;
            }
        }
        RegKey(HKEY hkey,const char16_t *subkey){
            if(RegOpenKeyW(hkey,reinterpret_cast<const wchar_t*>(subkey),&key) == ERROR_SUCCESS){
                opened = true;
            }
            else{
                opened = false;
            }
        }
        RegKey(const RegKey &) = delete;
        ~RegKey(){
            if(opened){
                RegCloseKey(key);
            }
        }
        /**
         * @brief Query the value
         * 
         * @param key The key ('\0' terminated)
         * @param outbuf The output buffer
         * @return LSTATUS 
         */
        LSTATUS query_value(std::u16string_view k,std::u16string &outbuf){
            DWORD size = outbuf.size();
            return RegQueryValueExW(
                key,
                reinterpret_cast<LPCWSTR>(k.data()),
                nullptr,
                nullptr,
                reinterpret_cast<LPBYTE>(outbuf.data()),
                &size
            );
        }
        /**
         * @brief Is succeed to open?
         * 
         * @return true 
         * @return false 
         */
        bool ok() const noexcept{
            return opened;
        }
        operator HKEY() const noexcept{
            return key;
        }
        HKEY key;
        bool opened;
    };
    //Get native context
    struct Context{
        HWND window;
        HDC hdc;
        HINSTANCE hinstance;
    };
    Context GetContext(SDL_Window*);
    /**
     * @brief Internal MessageHook
     * 
     * @param hWnd 
     * @param message 
     * @param wParam 
     * @param lParam 
     */
    void SDLCALL MessageHook(void *,void *hWnd,UINT message,UINT64 wParam,INT64 lParam);
}
}


#endif // _BTK_WIN32_INTERNAL_HPP_
