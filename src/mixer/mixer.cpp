#include "../build.hpp"

#include <Btk/impl/mixer.hpp>
#include <Btk/exception.hpp>
#include <Btk/mixer.hpp>
#include <Btk/Btk.hpp>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>
namespace Btk{
    static void init_audio(){
        if(SDL_WasInit(SDL_INIT_AUDIO) != SDL_INIT_AUDIO){
            if(SDL_Init(SDL_INIT_AUDIO) == -1){
                throwAudioError();
            }
        }
    }
    u8string GetAudioDevice(Uint32 idx,AudioDeviceType type){
        const char *s = SDL_GetAudioDeviceName(idx,type == Output);
        if(s == nullptr){
            throwAudioError();
        }
        return s;
    }
    Uint32 GetNumAudioDevices(AudioDeviceType type){
        init_audio();
        int i = SDL_GetNumAudioDevices(type == Output);
        if(i < 0){
            SDL_Init(SDL_INIT_AUDIO);

            throwAudioError();
        }
        return i;
    }
}

namespace Btk{
    AudioError::AudioError(const char *msg):
        std::runtime_error(msg){}

    
    AudioError::~AudioError(){}
    [[noreturn]] void throwAudioError(){
        throw AudioError(SDL_GetError());
    }
}
namespace Btk{
    AudioDeviceImpl::~AudioDeviceImpl(){

        SDL_CloseAudioDevice(dev);
        SDL_FreeAudioStream(stream);
        SDL_free(buffer);
    }
    static constexpr float MIX_MAXVOLUME = SDL_MIX_MAXVOLUME;
    void AudioDeviceImpl::run(Uint8 *dev_buf,int len){
        std::lock_guard<SpinLock> locker(stream_lock);

        int n = SDL_AudioStreamAvailable(stream);
        if(n == 0){
            SDL_AudioStreamFlush(stream);
        }
        n = SDL_AudioStreamAvailable(stream);
        //Extend the buffer
        if(n > bufsize){
            buffer = (Uint8*)SDL_realloc(buffer,n);
            bufsize = n;
        }

        if(SDL_AudioStreamGet(stream,buffer,n) == -1){
            //Error
            std::memset(dev_buf,dev_spec.silence,len);
            return;
        }
        //Write the buffer into device_buffer
        SDL_MixAudio(dev_buf,buffer,n,MIX_MAXVOLUME * volume / 100.0f);
        if(n < len){
            //Put slience
            std::memset(dev_buf + n,dev_spec.silence,len - n);
        }
    }
    void AudioDeviceImpl::open(const AudioDeviceInfo &info){
        SDL_zero(ipt_spec);
        ipt_spec.callback = Entry;
        ipt_spec.userdata = this;
        ipt_spec.format = info.format;
        ipt_spec.channels = info.channels;
        ipt_spec.freq = info.frequency;

        dev = SDL_OpenAudioDevice(
            info.name.c_str(),
            info.type == Input,
            &ipt_spec,
            &dev_spec,
            0
        );
        if(dev == 0){
            throwAudioError();
        }
        SDL_LockAudioDevice(dev);
        //TODOMake audio stream

        SDL_UnlockAudioDevice(dev);
    }
    void AudioDeviceImpl::Entry(void *self,Uint8 *dev_buf,int len){
        return static_cast<AudioDeviceImpl*>(self)->run(dev_buf,len);
    }



    AudioDevice::AudioDevice(){
        device = new AudioDeviceImpl();
    }
    AudioDevice::~AudioDevice(){
        delete device;
    }
    void AudioDevice::open(const AudioDeviceInfo &info){
        return device->open(info);
    }
}