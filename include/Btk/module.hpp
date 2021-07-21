#if !defined(_BTK_MODULE_HPP_)
#define _BTK_MODULE_HPP_
#include "string.hpp"
#include "defs.hpp"

#define BTK_MODULE_INIT() extern "C" void BtkModule_Init(Btk::Module& mod)
#define BTK_MODULE_QUIT() extern "C" void BtkModule_Quit(Btk::Module& mod)

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
