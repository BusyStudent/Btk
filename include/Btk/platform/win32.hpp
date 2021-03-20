#if !defined(_BTK_PLATFORM_WIN32)
#define _BTK_PLATFORM_WIN32
#define NOMINMAX
#include <windows.h>
#include <SDL2/SDL_events.h>
#include <string>
#include "../defs.hpp"
#include "../exception.hpp"
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
    BTKAPI std::string   StrMessageA(DWORD errcode);
    /**
     * @brief Errcode to string
     * 
     * @param errcode Windows error code
     * @return std::u16string
     */
    BTKAPI std::u16string StrMessageW(DWORD errcode);
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
