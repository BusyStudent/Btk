#if !defined(_BTK_IMPL_CODEC_HPP_)
#define _BTK_IMPL_CODEC_HPP_
#include <SDL2/SDL_surface.h>
#include "../defs.hpp"
#include "../pixels.hpp"
namespace Btk{
    /**
     * @brief Image Adpater for Laoding or Saving Image
     * 
     */
    struct ImageAdapter{
        /**
         * @brief Load the image
         * @param rwops
         * @return nullptr on failure
         */
        SDL_Surface *(*fn_load)(SDL_RWops *) = nullptr;
        /**
         * @brief Save the image
         * 
         * @param rwops
         * @param surf 
         * @param quality
         * @return true 
         * @return false 
         */
        bool (*fn_save)(SDL_RWops *,SDL_Surface *surf,int quality) = nullptr;
        /**
         * @brief Is the image
         * 
         * @return true 
         * @return false 
         */
        bool (*fn_is)(SDL_RWops *) = nullptr;

        ImageEncoder *(*create_encoder)() = nullptr;
        ImageDecoder *(*create_decoder)() = nullptr;

        SDL_Surface *load(SDL_RWops *rw) const{
            if(fn_load != nullptr){
                return fn_load(rw);
            }
            return nullptr;
        }
        bool is(SDL_RWops *rwops){
            if(fn_is != nullptr){
                return fn_is(rwops);
            }
            return false;
        }
        const char *name = nullptr;
        const char *vendor = nullptr;
    };
    BTKAPI void RegisterImageAdapter(const ImageAdapter &);
}

#endif // _BTK_IMPL_CODEC_HPP_
