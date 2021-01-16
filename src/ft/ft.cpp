#include "../build.hpp"

#include <SDL2/SDL_mutex.h>
#include <ft2build.h>

#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_STROKER_H
#include FT_GLYPH_H
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
            BTK_ASSERT(SDL_UnlockMutex(mutex) != -1);
        }
    };

    void Init(){
        if(library != nullptr){
            return;
        }
        if(FT_Init_FreeType(&library) == -1){
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

};