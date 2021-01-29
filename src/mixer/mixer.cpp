#include "../build.hpp"

#include <Btk/impl/mixer.hpp>
#include <Btk/exception.hpp>
#include <Btk/mixer.hpp>
#include <Btk/Btk.hpp>
#include <SDL2/SDL.h>
namespace Btk{
namespace Mixer{
    Library *library = nullptr;
    Music::~Music(){
        delete pimpl;
    }
}
}

namespace Btk{
namespace Mixer{
    void Init(){
        if(library == nullptr){
            if(SDL_Init(SDL_INIT_AUDIO) == -1){
                throwSDLError();
            }
            library = new Library;
            #ifndef NDEBUG
            fprintf(stderr,"Init Audio succeed\n");
            int n = SDL_GetNumAudioDevices(0);
            for(int i = 0;i < n;n ++){
                fprintf(stderr,"  Available device:%s",SDL_GetAudioDeviceName(i,0));
            }
            #endif
        }
    }
    void Quit(){
        if(library != nullptr){
            if(library->dylib_mp3 != nullptr){
                SDL_UnloadObject(library->dylib_mp3);
            }
            if(library->dylib_ogg != nullptr){
                SDL_UnloadObject(library->dylib_ogg);
            }
            if(library->dylib_flac != nullptr){
                SDL_UnloadObject(library->dylib_flac);
            }
            delete library;
            library = nullptr;
        }
    }
    bool WasInit(){
        return library != nullptr;
    }
}
}

namespace Btk{
    //Delete the device
    AudioDeviceImpl::~AudioDeviceImpl(){
        //Pause the device
        SDL_PauseAudioDevice(dev,0);
        SDL_CloseAudioDevice(dev);
    }
    void AudioDeviceImpl::close(){
        SDL_CloseAudioDevice(dev);
        current = nullptr;
    }
    bool AudioDeviceImpl::open(const char *device,SDL_AudioSpec *want){
        auto new_dev = SDL_OpenAudioDevice(device,0,&dev_spec,want,0);
        if(new_dev != 0){
            //succeed
            close();
            dev = new_dev;
            return true;
        }
        else{
            return false;
        }
    }
}
namespace Btk{

}

namespace Btk{
    MixerError::MixerError(const char *msg):
        std::runtime_error(msg){}

    
    MixerError::~MixerError(){}
    [[noreturn]] void throwMixerError(){
        throw MixerError(Mixer::GetError());
    }
}