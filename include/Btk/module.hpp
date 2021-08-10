#if !defined(_BTK_MODULE_HPP_)
#define _BTK_MODULE_HPP_
#include "string.hpp"
#include "defs.hpp"

/**
 * @brief Make the module entry
 * 
 */
#define BTK_MODULE_INIT(NAME) \
    extern "C" BTKEXPORT void BtkModule_Init(Btk::Module& NAME)
#define BTK_MODULE_QUIT(NAME) \
    extern "C" BTKEXPORT void BtkModule_Quit(Btk::Module& NAME)

namespace Btk{
    struct Module{
        typedef void (*QuitFn)(Module &);
        typedef void (*InitFn)(Module &);
        InitFn init = nullptr;
        QuitFn quit = nullptr;

        void *data = nullptr;//< Private data for Module
        void *handle = nullptr;
        u8string_view name;
        void unload();
    };
    BTKAPI void LoadModule(u8string_view module_name);
    BTKAPI bool HasModule(u8string_view module_name);
};


#endif // _BTK_MODULE_HPP_
