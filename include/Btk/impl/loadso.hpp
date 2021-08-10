#if !defined(_BTK_IMPL_LOADSO_HPP_)
#define _BTK_IMPL_LOADSO_HPP_
/**
 * @brief Header for Load Library
 * 
 */
#include <SDL2/SDL_loadso.h>
#include "../exception.hpp"
#include "../Btk.hpp"

namespace Btk::Impl{
    struct LibLoader{
        LibLoader(const char *soname){
            lib = SDL_LoadObject(soname);
            if(lib == nullptr){
                throwSDLError();
            }
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
            void *addr = SDL_LoadFunction(lib,name);
            if(addr == nullptr){
                throwSDLError();
            }
            return addr;
        }
    };
    template<class T>
    struct LibHolder{
        alignas(T) Uint8 buffer[sizeof(T)];
        bool is_loaded = false;
        void load(){
            if(not is_loaded){
                new(reinterpret_cast<T*>(buffer)) T;
                is_loaded = true;
                //Register Destrcutor
                AtExit(&LibHolder::unload,this);
            }
        }
        void unload(){
            if(is_loaded){
                reinterpret_cast<T*>(buffer)->~T();
                is_loaded = false;
            }
        }
        T *operator ->(){
            return reinterpret_cast<T*>(buffer);
        }
    };
    template<class T>
    struct ConstLibHolder:public T{
        static void load(){

        }
        static void unload(){

        }
        T *operator ->(){
            return reinterpret_cast<T*>(this);
        }
        const T *operator ->() const{
            return reinterpret_cast<const T*>(this);
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

#define BTK_MAKE_DYLIB(TYPE,NAME) Btk::Impl::LibHolder<TYPE> NAME;
#define BTK_MAKE_STLIB(TYPE,NAME) inline constexpr Btk::Impl::ConstLibHolder<TYPE> NAME;

#endif // _BTK_IMPL_LOADSO_HPP_
