#if !defined(_BTK_IMPL_MIXER)
#define _BTK_IMPL_MIXER
//Implment for mixer
#include <SDL2/SDL_audio.h>
namespace Btk{
namespace Mixer{
    class  Music;
    struct MusicImpl{
        
        virtual ~MusicImpl(){}
        Music *master;//The master of the impl
    };
    /**
     * @brief Mixer private librarys
     * 
     */
    struct Library{
        //audio librarys
        void *lib_ogg;
        void *lib_mp3;
        void *lib_flac;

        SDL_AudioDeviceID device;
    };
}
}


#endif // _BTK_IMPL_MIXER
