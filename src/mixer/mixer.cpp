#include "../build.hpp"

#include <Btk/impl/mixer.hpp>
#include <Btk/exception.hpp>
#include <Btk/mixer.hpp>
#include <Btk/Btk.hpp>

#include <mutex>

#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>

#include <algorithm>

namespace Btk{
    static void init_audio(){
        if(SDL_WasInit(SDL_INIT_AUDIO) != SDL_INIT_AUDIO){
            if(SDL_Init(SDL_INIT_AUDIO) == -1){
                throwAudioError();
            }
        }
    }
    u8string GetAudioDevice(Uint32 idx,AudioDeviceType type){
        const char *s = SDL_GetAudioDeviceName(idx,type == Input);
        if(s == nullptr){
            throwAudioError();
        }
        return s;
    }
    Uint32 GetNumAudioDevices(AudioDeviceType type){
        init_audio();
        int i = SDL_GetNumAudioDevices(type == Input);
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
    AudioDevice::AudioDevice(){

    }
    AudioDevice::~AudioDevice(){
        close();
    }
    void AudioDevice::close(){
        if(dev != 0){
            SDL_CloseAudioDevice(dev);
            dev = 0;
        }
        if(stream != nullptr){
            SDL_FreeAudioStream(stream);
            stream = nullptr;
        }
        if(buffer != nullptr){
            SDL_free(buffer);
            buffer = nullptr;
        }
    }
    void AudioDevice::open(const AudioDeviceInfo &info){
        SDL_zero(user_spec);
        user_spec.callback = [](void *self,Uint8 *buf,int len){
            try{
                static_cast<AudioDevice*>(self)->run(buf,len);
            }
            catch(...){
                DeferCall(std::rethrow_exception,std::current_exception());
            }
        };
        user_spec.userdata = this;
        user_spec.format = info.format;
        user_spec.channels = info.channels;
        user_spec.freq = info.frequency;
        user_spec.samples = info.samples;

        type = info.type;

        const char *name = nullptr;
        //If empty,Use default device
        if(not info.name.empty()){
            name = info.name.c_str();
        }

        dev = SDL_OpenAudioDevice(
            name,
            info.type == Input,
            &user_spec,
            &dev_spec,
            0
        );
        if(dev == 0){
            throwAudioError();
        }
        //TODOMake audio stream

        stream = SDL_NewAudioStream(
            info.format,
            info.channels,
            info.frequency,
            dev_spec.format,
            dev_spec.channels,
            dev_spec.freq
        );
        if(stream == nullptr){
            //Do cleanup
            SDL_UnlockAudioDevice(dev);
            close();
            throwAudioError();
        }
    }
    void AudioDevice::run(Uint8 *buf,int len){
        //Slience it
        memset(buf,dev_spec.silence,len);
        if(stream == nullptr){
            //Stream is un created yet
            return;
        }
        //Wait for the callback feed the data
        if(not _callback.empty()){
            _callback(*this,len);
        }
        // //Flush stream,Get data
        // SDL_AudioStreamFlush(stream);
        //Copy into buffer
        int ret = SDL_AudioStreamAvailable(stream);
        //We only need the size the fill the device buffer
        BTK_LOGINFO("[AudioThread::Worker]Buffer has %d byte data",ret);
        ret = std::clamp(ret,0,len);
        if(ret > buflen){
            //Reallocate our buffer
            buffer = (Uint8*)SDL_realloc(buffer,ret);
            BTK_ASSERT(buf != nullptr);
            buflen = ret;
        }
        //Copy into our buffer
        SDL_AudioStreamGet(stream,buffer,ret);
        //mix the device buffer
        float v = SDL_MIX_MAXVOLUME / 100.0f * volume;
        SDL_MixAudioFormat(buf,buffer,dev_spec.format,ret,v);
        //Done
    }
    //Write data
    void AudioDevice::write(const void *buf,size_t n){
        std::lock_guard locker(*this);
        if(SDL_AudioStreamPut(stream,buf,n) != 0){
            throwAudioError();
        }
    }
    void AudioDevice::flush(){
        std::lock_guard locker(*this);
        if(SDL_AudioStreamFlush(stream) != 0){
            throwAudioError();
        }
    }
    void AudioDevice::clear(){
        std::lock_guard locker(*this);
        SDL_AudioStreamClear(stream);
    }
    void AudioDevice::lock(){
        SDL_LockAudioDevice(dev);
    }
    void AudioDevice::unlock(){
        SDL_UnlockAudioDevice(dev);
    }
    void AudioDevice::pause(bool val){
        SDL_PauseAudioDevice(dev,val);
    }
    //Query buffer
    size_t AudioDevice::available(){
        std::lock_guard locker(*this);
        SDL_AudioStreamFlush(stream);
        return SDL_AudioStreamAvailable(stream);
    }
}