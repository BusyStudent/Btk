#include "../build.hpp"

#include <Btk/ft/ft.hpp>
#include <Btk/exception.hpp>

#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_rwops.h>

#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
#include FT_ERRORS_H
#include FT_TRUETYPE_IDS_H
//Btk freetype2 renderer
namespace BtkFt{
    
    FT_Library library = nullptr;
    SDL_mutex *mutex = nullptr;
    //We should lock it when using FT_New_Face and FT_Done_Face.

    /**
     * @brief A helper class to lock ft library
     * 
     */
    struct LockGuard{
        LockGuard(){
            SDL_LockMutex(mutex);
        }
        LockGuard(const LockGuard &) = delete;
        ~LockGuard(){
            int ret = SDL_UnlockMutex(mutex);
            BTK_ASSERT(ret != -1);
        }
    };

    void Init(){
        if(library != nullptr){
            return;
        }
        if(FT_Init_FreeType(&library) != FT_Err_Ok){
            //Handle err
        }
        mutex = SDL_CreateMutex();
    }
    void Quit(){
        if(library == nullptr){
            return;
        }
        FT_Done_FreeType(library);
        SDL_DestroyMutex(mutex);
        library = nullptr;
        mutex = nullptr;
    }
    
};
namespace BtkFt{
    
    Face::~Face(){
        LockGuard locker;
        FT_Done_Face(face);
    }
};