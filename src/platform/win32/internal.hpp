#if !defined(_BTK_WIN32_INTERNAL_HPP_)
#define _BTK_WIN32_INTERNAL_HPP_
#include <ShlObj.h>
#include <wingdi.h>
#include <winreg.h>

#include <string>

#define WIN32_WSTR (wchar_t*)
#define WIN32_CWSTR (const wchar_t *)

namespace Btk{
namespace Win32{
    /**
     * @brief A Helper class to manager com
     * 
     * @tparam T The com type
     */
    template<class T>
    struct ComInstance{
        ComInstance() = default;
        ComInstance(const ComInstance &) = delete;
        ~ComInstance(){
            release();
        }
        void release(){
            if(ptr != nullptr){
                ptr->Release();
            }
        }

        T *operator ->(){
            return ptr;
        }
        T **operator &(){
            return &ptr;
        }
        operator T *() const noexcept{
            return ptr;
        }
        T *ptr = nullptr;
    };
    /**
     * @brief A pointer to free memory from com
     * 
     * @tparam T The pointer type
     */
    template<class T>
    struct ComMemPtr{
        ComMemPtr(T *p = nullptr){
            ptr = p;
        }
        ComMemPtr(const ComMemPtr &) = delete;
        ~ComMemPtr(){
            CoTaskMemFree(ptr);
        }
        T *operator ->(){
            return ptr;
        }
        T **operator &(){
            return &ptr;
        }
        operator T*(){
            return ptr;
        }
        T *ptr;
    };
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
}
}


#endif // _BTK_WIN32_INTERNAL_HPP_
