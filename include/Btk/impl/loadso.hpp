#if !defined(_BTK_IMPL_LOADSO_HPP_)
#define _BTK_IMPL_LOADSO_HPP_
/**
 * @brief Header for Load Library
 * 
 */
#include <SDL2/SDL_loadso.h>
#include "../exception.hpp"

namespace Btk::Impl{
    struct LibLoader{
        LibLoader(const char *soname){
            lib = SDL_LoadObject(soname);
        }
        LibLoader(const LibLoader &) = delete;
        ~LibLoader(){
            SDL_UnloadObject(lib);
        }
        void *lib;
        /**
         * @brief Get function name
         * 
         * @param name 
         * @return void* 
         */
        void *find(const char *name) const{
            return SDL_LoadFunction(lib,name);
        }
    };
}

/**
 * @brief Load library
 * 
 */
#define BTK_DYMAIC_LIBRARY(LIBNAME) \
    Btk::Impl::LibLoader _loader = LIBNAME
#define BTK_STATIC_LIBRARY(LIBNAME)

#define BTK_DYMAIC_FUNCTION(FUNC) \
    decltype(::FUNC) *FUNC = reinterpret_cast<decltype(::FUNC) *>(_loader.find(#FUNC))
#define BTK_STATIC_FUNCTION(FUNC) \
    static constexpr auto FUNC = ::FUNC

#endif // _BTK_IMPL_LOADSO_HPP_
