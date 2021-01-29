#if !defined(_BTK_MIXER_HPP_)
#define _BTK_MIXER_HPP_
#include <string_view>
#include <stdexcept>
#include "defs.hpp"
#include "signal/signal.hpp"
namespace Btk{
namespace Mixer{
    struct MusicImpl;
    /**
     * @brief Music
     * 
     */
    class BTKAPI Music{
        public:
            ~Music();
            static Music FromFile(std::string_view fname);
        private:
            MusicImpl *pimpl;
    };
    BTKAPI void Init();
    BTKAPI void Quit();
    BTKAPI bool WasInit();
}
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
            AudioDevice();
            AudioDevice(const AudioDevice &) = delete;
            ~AudioDevice();
            void play(Mixer::Music &music);
        private:
            AudioDeviceImpl *pimpl;
        friend Mixer::Music;
    };
    #if 0
    using MixerMusic = Mixer::Music;
    using MixerAudio = Mixer::Music;
    using MixerChunk = Mixer::Chunk;
    using MixerChannal = Mixer::Channal;
    #endif
    [[noreturn]] void BTKAPI throwMixerError(const char *msg);
    [[noreturn]] void BTKAPI throwMixerError();
}

#endif // _BTK_MIXER_HPP_
