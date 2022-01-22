#if !defined(_BTK_PLATFORM_WIN32)
#define _BTK_PLATFORM_WIN32
#include <windows.h>
#include <combaseapi.h>
#include <SDL2/SDL_events.h>
#include "../defs.hpp"
#include "../string.hpp"
#include "../exception.hpp"

#include <map>

#undef max
#undef min
#undef MessageBox
#undef LoadImage
#undef CreateWindow

//CoCreateInstance Helper
#define Btk_CoCreateInstance(T) \
    Btk::Win32::ComCreateInstance<I##T>(\
        CLSID_##T,IID_I##T\
    )

namespace Btk{
    class WindowImpl;
    class RendererDevice;
}

namespace Btk{
namespace Win32{
    BTKAPI void Init();
    BTKAPI void Quit();
    BTKAPI void HandleSysMsg(const SDL_SysWMmsg &);
    /**
     * @brief Errcode to string
     * 
     * @param errcode Windows error code
     * @return std::string
     */
    BTKAPI u8string  StrMessageA(DWORD errcode);
    /**
     * @brief Errcode to string
     * 
     * @param errcode Windows error code
     * @return std::u16string
     */
    BTKAPI u16string StrMessageW(DWORD errcode);
    /**
     * @brief Win32 native MessageBox
     * 
     * @param title
     * @param msg 
     * @param flag 
     * @return BTKAPI 
     */
    BTKAPI bool MessageBox(u8string_view title,u8string_view msg,int flag = 0);
    /**
     * @brief Get the Window object by hwnd
     * 
     * @param hwnd description
     * 
     * @return nullptr on error
     */
    BTKAPI WindowImpl *GetWindow(HWND hwnd);
        /**
     * @brief A Helper class to manager com
     * 
     * @tparam T The com type
     */
    template<class T>
    struct ComInstance{
        ComInstance() = default;
        ComInstance(T *p):ptr(p){}
        ComInstance(const ComInstance &instance){
            ptr = instance.ptr;
            if(ptr != nullptr){
                ptr->AddRef();
            }
        }
        ~ComInstance(){
            release();
        }
        void release(){
            if(ptr != nullptr){
                ptr->Release();
            }
        }

        //Assign
        ComInstance &operator =(const ComInstance &instance){
            release();
            ptr = instance.ptr;
            if(ptr != nullptr){
                ptr->AddRef();
            }
        }
        ComInstance &operator =(ComInstance &&instance){
            release();
            ptr = instance.ptr;
            instance.ptr = nullptr;
        }
        ComInstance &operator =(T *p){
            release();
            ptr = p;
            return *this;
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
     * @brief For mapping hwnd to pointer
     * 
     * @tparam T The pointer(like void*)
     */
    template<class T>
    struct HWNDMap{
        std::map<void*,void*> mp;

        void insert(HWND hwnd,T pointer){
            mp[hwnd] = pointer;
        }
        void erase(HWND hwnd){
            mp.erase(hwnd);
        }
        T    find(HWND hwnd){
            auto iter = mp.find(hwnd);
            if(iter == mp.end()){
                return nullptr;
            }
            return reinterpret_cast<T>(iter->second);
        }
    };
    template<class T>
    ComInstance<T> ComCreateInstance(const IID &id,const IID &rrid){
        ComInstance<T> ins;
        HRESULT hr = CoCreateInstance(
            id,
            nullptr,
            CLSCTX_ALL,
            rrid,
            reinterpret_cast<LPVOID>(&ins)
        );
        if(SUCCEEDED(hr)){
            return ins;
        }
        throwRuntimeError("CoCreateInstance failed");
    }
}
}
namespace Btk{
    /**
     * @brief Windows error
     * 
     */
    class BTKAPI Win32Error:public RuntimeError{
        public:
            Win32Error(DWORD errcode);
            ~Win32Error();
            DWORD errcode;//< Error code
    };
    [[noreturn]] 
    void BTKAPI throwWin32Error(DWORD errocode = GetLastError());
    //Renderer Device
    BTKAPI
    RendererDevice *CreateD3D11Device(HWND hwnd);
}
#endif // _BTK_PLATFORM_WIN32
