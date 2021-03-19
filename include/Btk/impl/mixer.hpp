#if !defined(_BTK_IMPL_MIXER)
#define _BTK_IMPL_MIXER
//Implment for mixer
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_rwops.h>
#include "../mixer.hpp"
#include "../rwops.hpp"
#include <vector>
#include <mutex>
#include <list>

namespace Btk{
    struct AudioDeviceImpl{
        ~AudioDeviceImpl();
        SDL_AudioDeviceID dev;//< The device id
        SDL_AudioSpec dev_spec;//< The detail of the stream
        Mixer::MusicImpl *current;//< Currently playing music
        int volume = SDL_MIX_MAXVOLUME;//Default to max volume
        std::vector<Uint8> buffer;


        void lock();
        void unlock();

        void close();
        /**
         * @brief Open the audio device to output
         * 
         * @param dev The device name(nullptr)
         * @param want 
         * @return true on succeed
         * @return false on failure
         */
        bool open(const char *dev,SDL_AudioSpec *want);
    };
}
namespace Btk{
namespace Mixer{
    class  Music;
    struct MusicImpl{
        virtual ~MusicImpl(){};
        SDL_AudioSpec spec;//pcm formated data
        Music *master;//The master of the impl
        AudioDeviceImpl *dev;//The device belong to
    };
    struct AudioPlayerImpl{
        SDL_AudioStream *stream;//<AudioStream
        Uint32 device;//< Device ID
        MusicImpl *current;//< current play music
    };
    
    /**
     * @brief Mixer private librarys and audio workers
     * 
     */
    struct Library{
        //audio librarys
        void *dylib_ogg = nullptr;
        void *dylib_mp3 = nullptr;
        void *dylib_flac = nullptr;

        //Audio devices pointer
        std::list<AudioDeviceImpl*> devices;
        std::mutex mtx;
    };
    extern BTKAPI Library* library;
    /**
     * @brief Internal function for load WAV 
     * 
     * @internal User should not use it
     * @param rwops The rwops
     * @return MusicImpl* nullptr on error
     */
    MusicImpl *OpenWAV(SDL_RWops *rwops);
    
    template<class ...Args>
    int SetError(const char *fmt,Args &&...args){
        return SDL_SetError(fmt,std::forward<Args>(args)...);
    }
    inline const char *GetError(){
        return SDL_GetError();
    }
}
}


#endif // _BTK_IMPL_MIXER
