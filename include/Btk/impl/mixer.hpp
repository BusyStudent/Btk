#if !defined(_BTK_IMPL_MIXER)
#define _BTK_IMPL_MIXER
//Implment for mixer
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_rwops.h>
#include "../mixer.hpp"
#include "../rwops.hpp"
#include "atomic.hpp"
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
namespace Btk::Mixer{
    class  Music;
    struct MusicImpl;
    struct AudioPlayerImpl{
        ~AudioPlayerImpl();
        SDL_AudioStream *stream;//<AudioStream
        Uint32 device;//< Device ID
        MusicImpl *current;//< current play music

        int volume;
    };
    struct MusicImpl{
        virtual ~MusicImpl(){};
        /**
         * @brief Poll the data
         * 
         * @param data 
         * @param len 
         */
        //virtual void poll(AudioPlayerImpl*,Uint8 *data,int len) = 0;
        SDL_AudioSpec spec;//pcm formated data
        Music *master;//The master of the impl
        AudioDeviceImpl *dev;//The device belong to
        Atomic refcount;
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
        std::list<MusicImpl*(*)(SDL_RWops*)> adapter;
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
    BTKHIDDEN MusicImpl *OpenWAV(SDL_RWops *rwops);
    
    template<class ...Args>
    int SetError(const char *fmt,Args &&...args){
        return SDL_SetError(fmt,std::forward<Args>(args)...);
    }
    inline const char *GetError(){
        return SDL_GetError();
    }
    /**
     * @brief Open music from Rwops
     * 
     * @return MusicImpl* 
     */
    BTKHIDDEN MusicImpl *OpenMusic(SDL_RWops *);
}


#endif // _BTK_IMPL_MIXER
