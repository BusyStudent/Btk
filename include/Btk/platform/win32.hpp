#if !defined(_BTK_PLATFORM_WIN32)
#define _BTK_PLATFORM_WIN32
#define NOMINMAX
#include <windows.h>
#include <SDL2/SDL_events.h>
#include "../defs.hpp"
#include "../string.hpp"
#include "../exception.hpp"

#undef MessageBox
#undef LoadImage

namespace Btk{
    class WindowImpl;
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
    BTKAPI bool MessageBox(std::string_view title,std::string_view msg,int flag = 0);
    /**
     * @brief Get the Window object by hwnd
     * 
     * @param hwnd description
     * 
     * @return nullptr on error
     */
    BTKAPI WindowImpl *GetWindow(HWND hwnd);
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
            /**
             * @brief Get the error string
             * 
             * @return const char* 
             */
            const char *what() const noexcept;
    };
    [[noreturn]] void BTKAPI throwWin32Error(DWORD errocode = GetLastError());
}
#endif // _BTK_PLATFORM_WIN32
