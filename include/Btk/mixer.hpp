#if !defined(_BTK_MIXER_HPP_)
#define _BTK_MIXER_HPP_
#include <SDL2/SDL_audio.h>
#include <stdexcept>
#include "defs.hpp"
#include "signal.hpp"
#include "string.hpp"

#define BTK_AUDIO_FMT(X) X = AUDIO_##X

namespace Btk::Mixer{
    struct MusicImpl;
    struct AudioPlayerImpl;
    /**
     * @brief Music
     * 
     */
    class BTKAPI Music{
        public:
            ~Music();
            static Music FromFile(u8string_view fname);
        private:
            MusicImpl *pimpl;
    };
    class BTKAPI AudioPlayer{
        public:
            AudioPlayer();
            AudioPlayer(const AudioPlayer &) = delete;
            ~AudioPlayer();
        private:
            AudioPlayerImpl *pimpl;
    };
    BTKAPI void Init();
    BTKAPI void Quit();
    BTKAPI bool WasInit();
}


namespace Btk{
    struct AudioDeviceImpl;
    /**
     * @brief Audio exception
     * 
     */
    class BTKAPI AudioError:public std::runtime_error{
        public:
            AudioError(const char *msg);
            AudioError(const AudioError &) = default;
            ~AudioError();
    };
    using AudioDeviceName = u8string;
    /**
     * @brief Type of the device
     * 
     */
    enum AudioDeviceType:Uint8{
        Input,
        Output
    };
    /**
     * @brief AudioFormat for device
     * 
     */
    enum AudioFormat:Uint16{
        //8 samples
        BTK_AUDIO_FMT(U8),
        BTK_AUDIO_FMT(S8),
        //U16
        BTK_AUDIO_FMT(U16),
        BTK_AUDIO_FMT(U16LSB),
        BTK_AUDIO_FMT(U16MSB),
        BTK_AUDIO_FMT(U16SYS),
        //S16
        BTK_AUDIO_FMT(S16),
        BTK_AUDIO_FMT(S16LSB),
        BTK_AUDIO_FMT(S16MSB),
        BTK_AUDIO_FMT(S16SYS),
        //INT32
        BTK_AUDIO_FMT(S32),
        BTK_AUDIO_FMT(S32LSB),
        BTK_AUDIO_FMT(S32MSB),
        BTK_AUDIO_FMT(S32SYS),
        //FLOAT
        BTK_AUDIO_FMT(F32),
        BTK_AUDIO_FMT(F32LSB),
        BTK_AUDIO_FMT(F32MSB),
        BTK_AUDIO_FMT(F32SYS),
        //Alias
        u8 = U8,
        s8 = S8,
        u16 = U16,
        s16 = S16,
        s32 = S32,
        f32 = F32,
    };
    /**
     * @brief Detail of audio
     * 
     */
    struct AudioInfo{
        AudioFormat format;
        Uint8 channels;
        int frequency;
    };
    /**
     * @brief Detail of audio device
     * 
     */
    struct AudioDeviceInfo:public AudioInfo{
        AudioDeviceName name;
        AudioDeviceType type = Output;

    };
    class BTKAPI AudioDevice{
        public:
            /**
             * @brief Construct a new empty Audio Device object
             * 
             */
            AudioDevice();
            AudioDevice(const AudioDevice &) = delete;
            ~AudioDevice();

            /**
             * @brief Open audio device
             * 
             * @param info 
             */
            void open(const AudioDeviceInfo &info);
            /**
             * 
             * @brief Close audio device
             * 
             */
            void close();

        private:
            AudioDeviceImpl *device;
        friend Mixer::Music;
    };
    #if 0
    using MixerMusic = Mixer::Music;
    using MixerAudio = Mixer::Music;
    using MixerChunk = Mixer::Chunk;
    using MixerChannal = Mixer::Channal;
    #endif
    using Mixer::AudioPlayer;
    [[noreturn]] void BTKAPI throwAudioError(const char *msg);
    [[noreturn]] void BTKAPI throwAudioError();

    /**
     * @brief Get the Audio Device name
     * 
     * @param idx The audio device index
     * @param type The audio device type
     * @return u8string 
     */
    [[nodiscard]]
    u8string   BTKAPI GetAudioDevice(Uint32 idx,AudioDeviceType type = Output);
    /**
     * @brief Get the Num Audio Devices
     * 
     * @param type 
     * @return Uint32 
     */
    [[nodiscard]]
    Uint32     BTKAPI GetNumAudioDevices(AudioDeviceType type = Output);
    
    /**
     * @brief List All audio device
     * 
     * @param type 
     * @return BTKAPI 
     */
    [[nodiscard]]
    StringList inline ListAudioDevices(AudioDeviceType type = Output){
        StringList devices;
        Uint32 num = GetNumAudioDevices(type);
        for(Uint32 i = 0;i < num;i++){
            devices.push_back(GetAudioDevice(i,type));
        }
        return devices;
    }
}

#endif // _BTK_MIXER_HPP_
