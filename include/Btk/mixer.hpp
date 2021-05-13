#if !defined(_BTK_MIXER_HPP_)
#define _BTK_MIXER_HPP_
#include <string_view>
#include <stdexcept>
#include "defs.hpp"
#include "signal.hpp"
#include "string.hpp"
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
     * @brief Mixer's exception
     * 
     */
    class BTKAPI MixerError:public std::runtime_error{
        public:
            MixerError(const char *msg);
            MixerError(const MixerError &) = default;
            ~MixerError();
    };
    class AudioDevice{
        public:
            /**
             * @brief SDL Device Index
             * 
             */
            struct Index{
                int index;
            };
            /**
             * @brief Construct a new empty Audio Device object
             * 
             */
            AudioDevice():dev(0){};
            AudioDevice(const AudioDevice &) = delete;
            ~AudioDevice();

            Uint32 get() const noexcept{
                return dev;
            }
            /**
             * @brief Open audio device
             * 
             * @param dev_index 
             */
            void open(Index dev_index);
            /**
             * 
             * @brief Close audio device
             * 
             */
            void close();

        private:
            Uint32 dev;//< SDL_DeviceID
        friend Mixer::Music;
    };
    #if 0
    using MixerMusic = Mixer::Music;
    using MixerAudio = Mixer::Music;
    using MixerChunk = Mixer::Chunk;
    using MixerChannal = Mixer::Channal;
    #endif
    using Mixer::AudioPlayer;
    [[noreturn]] void BTKAPI throwMixerError(const char *msg);
    [[noreturn]] void BTKAPI throwMixerError();
}

#endif // _BTK_MIXER_HPP_
