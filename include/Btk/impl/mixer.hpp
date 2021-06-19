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
    struct BTKHIDDEN AudioDeviceImpl{
        AudioDeviceImpl() = default;
        AudioDeviceImpl(const AudioDeviceImpl &) = delete;
        ~AudioDeviceImpl();

        void open(const AudioDeviceInfo &info);

        SDL_AudioDeviceID dev = 0;
        SDL_AudioStream *stream = nullptr;
        SpinLock stream_lock;
        SDL_AudioSpec dev_spec;//< Device spec
        SDL_AudioSpec ipt_spec;//< Input spec
        //Buffer
        Uint8 *buffer = nullptr;
        size_t bufsize = 0;

        float volume = 100.0f;

        void run(Uint8 *dev_buf,int len);
        static void SDLCALL Entry(void *self,Uint8 *buffer,int len);
    };
}

#endif // _BTK_IMPL_MIXER
