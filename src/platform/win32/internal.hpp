#if !defined(_BTK_WIN32_INTERNAL_HPP_)
#define _BTK_WIN32_INTERNAL_HPP_
#include <ShlObj.h>
#include <wingdi.h>
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
}
}


#endif // _BTK_WIN32_INTERNAL_HPP_
